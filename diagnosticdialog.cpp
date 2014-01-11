#include "diagnosticdialog.h"
#include "ui_diagnosticdialog.h"
#include <QTemporaryFile>
#include <QTextDocumentWriter>
#include <QFileDialog>
#include <QPushButton>

extern QTemporaryFile gLogFile;

DiagnosticDialog::DiagnosticDialog(QWidget *parent) :
    QDialog(parent),_filename(""),
    ui(new Ui::DiagnosticDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Diagnostic");

    QFile* log_file = new QFile(gLogFile.fileName());
    log_file->open(QIODevice::ReadOnly);
    QString log(log_file->readAll());
    log_file->close();

    _vboxlayout = new QVBoxLayout(this);

    _log_textedit  = new QTextEdit();
    _log_textedit->setText(log);
    _log_textedit->setReadOnly(true);

    _buttonbox = new QDialogButtonBox();
    _ok_button = new QPushButton(tr("&OK"));
    _save_button = new QPushButton(tr("&Save"));

    _buttonbox->addButton(_ok_button, QDialogButtonBox::AcceptRole);
    _buttonbox->addButton(_save_button,QDialogButtonBox::ActionRole);
    _ok_button->setAutoDefault(true);

    QObject::connect(_ok_button, SIGNAL(clicked()),  this, SLOT(onOkButtonClicked()));
    QObject::connect(_save_button, SIGNAL(clicked()),  this, SLOT(onSaveButtonClicked()));

    _vboxlayout->addWidget(_log_textedit);
    _vboxlayout->addWidget(_buttonbox);

    setLayout(_vboxlayout);
    delete log_file;
    log_file = nullptr;
}

DiagnosticDialog::~DiagnosticDialog()
{
    delete ui;
}

void DiagnosticDialog::onOkButtonClicked()
{
        this->close();
        return;
}

void DiagnosticDialog::onSaveButtonClicked()
{
    if( _filename.isEmpty() )
        _filename = QFileDialog::getSaveFileName( this, tr("Save Diagnostic"), "." ,tr("All Files (*)"));

    QTextDocumentWriter writer(_filename);
    writer.write(_log_textedit->document());
}
