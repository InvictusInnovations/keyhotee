#pragma once
#include <QtGui>
#include <bts/profile.hpp>

class MailboxModel;

typedef std::pair<MailboxModel*, QModelIndex> TMailMsgIndex;

class MailboxModelRoot
{
public:
  /** Class constructor.
  */
  MailboxModelRoot();
  ~MailboxModelRoot();

  void addMailboxModel(MailboxModel* model);
  TMailMsgIndex findSrcMail(const fc::sha256 &digest);

private:
  typedef std::list<MailboxModel*>  TMailboxModelList;
  
  TMailboxModelList                 _models_list;
};
