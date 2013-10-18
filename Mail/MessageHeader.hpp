#pragma once
#include <QtGui>

class MessageHeader
{
    public:
       MessageHeader():read_mark(false),attachment(false),money_amount(0){}

       QString     from;
       QIcon       from_icon;
       QString     to;
       QString     subject;
       QDateTime   date_received;
       QDateTime   date_sent;
       bool        read_mark;
       bool        attachment;
       QIcon       money_type;
       double      money_amount;
       QString     body;

       fc::uint256  digest;
};

