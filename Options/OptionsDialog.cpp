#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

#include <QSettings>
#include <QDirIterator>

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::OptionsDialog)
{
  ui->setupUi(this);

  fillLanguageComboBox();

  QSettings globa_settings("Invictus Innovations", "Keyhotee");
  QString loaded_profile_name = globa_settings.value("last_profile").toString();
  
  _settings_file = "keyhotee_";
  _settings_file.append(loaded_profile_name);
  QSettings settings("Invictus Innovations", _settings_file);

  QLocale locale = QLocale(settings.value("Language", "").toString());
  _lang_id = ui->language->findData(locale.name());
  ui->language->setCurrentIndex(_lang_id);

  ui->chat_authorized->setChecked(settings.value("AllowChat", "").toBool());
  ui->mail_authorized->setChecked(settings.value("AllowMail", "").toBool());

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
  QSettings settings("Invictus Innovations", _settings_file);

  // General
  QString localeName = ui->language->currentData().toString();
  settings.setValue("Language", localeName);
  int index = ui->language->currentIndex();

  bool lang_changed = (_lang_id != index);

  // Privacy
  if (ui->chat_authorized->isChecked())
    settings.setValue("AllowChat", true);
  else if (ui->chat_contacts->isChecked())
    settings.setValue("AllowChat", false);

  if (ui->mail_authorized->isChecked())
    settings.setValue("AllowMail", true);
  else if (ui->mail_anyone->isChecked())
    settings.setValue("AllowMail", false);
  
  emit optionsSaved(lang_changed);
}
