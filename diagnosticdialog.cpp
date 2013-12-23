#include "diagnosticdialog.h"
#include "ui_diagnosticdialog.h"
#include <QTemporaryFile>
#include <QTextDocumentWriter>
#include <QFileDialog>

extern QTemporaryFile gLogFile;

DiagnosticDialog::DiagnosticDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiagnosticDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Diagnostic");
    QFile* log_file = new QFile(gLogFile.fileName());
    log_file->open(QIODevice::ReadWrite);

    QString log(log_file->readAll());
    log_file->close();
    ui->textEdit->setPlainText(log);
    ui->textEdit->show();
}

DiagnosticDialog::~DiagnosticDialog()
{
    delete ui;
}

void DiagnosticDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button->text() == "&OK")
    {
        this->close();
        return;
    }
    else if(button->text() == "&Save")
    if( fileName.isEmpty() )
        fileName = QFileDialog::getSaveFileName( this, tr("Save Diagnostic"), "." ,tr("All Files (*)"));

    QTextDocumentWriter writer(fileName);
    writer.write(ui->textEdit->document());

}
