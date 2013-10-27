#pragma once
#include <QtGui>
#include <bts/bitchat/bitchat_message_db.hpp>
#include <fc/crypto/elliptic.hpp>

class MessageHeader
{
    public:
       MessageHeader():attachment(false),money_amount(0){}

       bts::bitchat::message_header header;
       QString     from;
       QIcon       from_icon;
       std::vector<fc::ecc::public_key>   to_list;
       std::vector<fc::ecc::public_key>   cc_list;
       QString     subject;
       QDateTime   date_received;
       QDateTime   date_sent;
       bool        attachment;
       QIcon       money_type;
       double      money_amount;
       QString     body;
};

