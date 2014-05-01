#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OptionsDialog(QWidget *parent, QString profile_name);
  ~OptionsDialog();

signals:
  void optionsSaved(bool lang_changed);

private:
  void fillLanguageComboBox();
  void onSave();

private:
  Ui::OptionsDialog *ui;

  QString           _settings_file;
  int               _lang_id;
};

#endif // OPTIONSDIALOG_H
