#include "MailInbox.hpp"
#include "../ui_MailInbox.h"

MailInbox::MailInbox( QWidget* parent )
:ui( new Ui::MailInbox() )
{
   ui->setupUi( this );
}

MailInbox::~MailInbox()
{
}
void MailInbox::setModel( QAbstractItemModel* m )
{
   ui->inbox_table->setModel(m);
   ui->inbox_table->horizontalHeader()->resizeSection( 4, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( 5, 300 );
   ui->inbox_table->horizontalHeader()->resizeSection( 6, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( 7, 120 );
   ui->inbox_table->horizontalHeader()->setSectionsMovable(true);
   ui->inbox_table->horizontalHeader()->setSortIndicatorShown(false);
   ui->inbox_table->horizontalHeader()->setSectionsClickable(true);
   ui->inbox_table->horizontalHeader()->setHighlightSections(true);
}



