#include "MailInbox.hpp"
#include "../ui_MailInbox.h"

MailInbox::MailInbox( QWidget* parent )
:ui( new Ui::MailInbox() )
{
   ui->setupUi( this );
   ui->inbox_table->verticalHeader()->resizeSection( 4, 120 );
   ui->inbox_table->verticalHeader()->resizeSection( 5, 300 );
   ui->inbox_table->verticalHeader()->resizeSection( 6, 120 );
   ui->inbox_table->verticalHeader()->resizeSection( 7, 120 );
}

MailInbox::~MailInbox()
{
}
void MailInbox::setModel( QAbstractItemModel* m )
{
   ui->inbox_table->setModel(m);
   ui->inbox_table->verticalHeader()->resizeSection( 4, 120 );
   ui->inbox_table->verticalHeader()->resizeSection( 5, 300 );
   ui->inbox_table->verticalHeader()->resizeSection( 6, 120 );
   ui->inbox_table->verticalHeader()->resizeSection( 7, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( 4, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( 5, 300 );
   ui->inbox_table->horizontalHeader()->resizeSection( 6, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( 7, 120 );
}



