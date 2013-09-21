
#include "MailViewer.hpp"
#include "../ui_MailViewer.h"

MailViewer::MailViewer( QWidget* parent )
:ui( new Ui::MailViewer() )
{
   ui->setupUi( this );
}

MailViewer::~MailViewer()
{
}

