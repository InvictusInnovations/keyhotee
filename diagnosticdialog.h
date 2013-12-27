#ifndef DIAGNOSTICDIALOG_H
#define DIAGNOSTICDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>

namespace Ui {
class DiagnosticDialog;
}

class DiagnosticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiagnosticDialog(QWidget *parent = 0);
    ~DiagnosticDialog();

private slots:

    void onOkButtonClicked();
    void onSaveButtonClicked();

private:
    QTextEdit*          _log_textedit;
    QDialogButtonBox*      _buttonbox;
    QVBoxLayout*          _vboxlayout;
    QPushButton*           _ok_button;
    QPushButton*         _save_button;


    QString                 _filename;
    Ui::DiagnosticDialog          *ui;
};

#endif // DIAGNOSTICDIALOG_H
