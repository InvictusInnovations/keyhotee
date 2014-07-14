#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

#include <QSettings>
#include <QDirIterator>

OptionsDialog::OptionsDialog(QWidget *parent, QString profile_name) :
  QDialog(parent),
  ui(new Ui::OptionsDialog)
{
  ui->setupUi(this);

  fillLanguageComboBox();

  QSettings globa_settings("Invictus Innovations", "Keyhotee");
  
  QLocale locale = QLocale(globa_settings.value("Language", "").toString());
  _lang_id = ui->language->findData(locale.name());
  ui->language->setCurrentIndex(_lang_id);

  _settings_file = "keyhotee_";
  _settings_file.append(profile_name);
  QSettings profile_settings("Invictus Innovations", _settings_file);
  ui->enable_filter_blocked->setChecked(profile_settings.value("FilterBlocked", "").toBool());
  ui->chat_authorized->setChecked(profile_settings.value("AllowChat", "").toBool());
  ui->mail_authorized->setChecked(profile_settings.value("AllowMail", "").toBool());
  ui->save_spam->setChecked(profile_settings.value("SaveSpam", "").toBool());
  ui->wallets_client_on_startup->setChecked(profile_settings.value("BitSharesClientOnStartup", "").toBool());

  connect(this, &QDialog::accepted, this, &OptionsDialog::onSave);
}

OptionsDialog::~OptionsDialog()
{
  delete ui;
}

void OptionsDialog::fillLanguageComboBox()
{
  QStringList available_languages = QStringList("en_US");
  QDirIterator qm_it(":/", QStringList() << "*.qm", QDir::Files);
  while (qm_it.hasNext())
  {
    qm_it.next();
    available_languages << qm_it.fileName().remove("keyhotee_").remove(".qm");
  }

  foreach(QString language, available_languages)
  {
    QLocale locale = QLocale(language);
    QString native_language = locale.nativeLanguageName();
    QString en_language = QLocale::languageToString(locale.language());
    QString text = native_language + " (" + en_language + ")";

    ui->language->addItem(text, locale.name());
  }

  ui->language->model()->sort(0);
}

void OptionsDialog::onSave()
{
  QSettings globa_settings("Invictus Innovations", "Keyhotee");

  // General
  QString localeName = ui->language->currentData().toString();
  globa_settings.setValue("Language", localeName);
  int index = ui->language->currentIndex();

  bool lang_changed = (_lang_id != index);

  QSettings profile_settings("Invictus Innovations", _settings_file);

  // General
  profile_settings.setValue("FilterBlocked", ui->enable_filter_blocked->isChecked());

  // Privacy
  if (ui->chat_authorized->isChecked())
    profile_settings.setValue("AllowChat", true);
  else if (ui->chat_contacts->isChecked())
    profile_settings.setValue("AllowChat", false);

  if (ui->mail_authorized->isChecked())
    profile_settings.setValue("AllowMail", true);
  else if (ui->mail_anyone->isChecked())
    profile_settings.setValue("AllowMail", false);

  if(ui->save_spam->isChecked())
    profile_settings.setValue("SaveSpam", true);
  else
    profile_settings.setValue("SaveSpam", false);

  // Wallets
  if(ui->wallets_client_on_startup->isChecked())
    profile_settings.setValue("BitSharesClientOnStartup", true);
  else
    profile_settings.setValue("BitSharesClientOnStartup", false);
  
  emit optionsSaved(lang_changed);
}
