#ifndef DIAGNOSTICDIALOG_H
#define DIAGNOSTICDIALOG_H

#include <QAbstractButton>
#include <QDialog>

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

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    QString fileName;
    Ui::DiagnosticDialog *ui;
};

#endif // DIAGNOSTICDIALOG_H
