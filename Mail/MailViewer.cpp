
#include "MailViewer.hpp"
#include "../ui_MailViewer.h"
#include <QToolBar>

MailViewer::MailViewer( QWidget* parent )
:ui( new Ui::MailViewer() )
{
   ui->setupUi( this );

   message_tools = new QToolBar( ui->toolbar_container ); 
   reply = new QAction( QIcon( ":/images/mail_reply.png"), tr( "Reply"), this );
   reply_all = new QAction( QIcon( ":/images/mail_reply_all.png"), tr( "Reply All"),this );
   forward = new QAction( QIcon( ":/images/mail_forward.png"), tr("Forward"), this);
   delete_mail = new QAction(QIcon( ":/images/delete_icon.png"), tr( "Delete" ), this);

   message_tools->addAction( reply );
   message_tools->addAction( reply_all );
   message_tools->addAction( forward );

   QWidget* spacer = new QWidget(message_tools);
   spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
   message_tools->addWidget(spacer);

   message_tools->addAction( delete_mail );

   QGridLayout* l = new QGridLayout(ui->toolbar_container);
   l->setContentsMargins( 0,0,0,0);
   l->setSpacing(0);
   ui->toolbar_container->setLayout(l);
   l->addWidget(message_tools,0,0);
}

MailViewer::~MailViewer()
{
}

