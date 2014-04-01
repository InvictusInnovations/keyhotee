#include "MailboxModelRoot.hpp"

#include "MailboxModel.hpp"

MailboxModelRoot::MailboxModelRoot()
{
}

MailboxModelRoot::~MailboxModelRoot()
{
}

void MailboxModelRoot::addMailboxModel(MailboxModel* model)
{
  _models_list.push_back(model);
}

TMailMsgIndex MailboxModelRoot::findSrcMail(const fc::sha256 &digest)
{
  TMailMsgIndex src_msg;

  TMailboxModelList::iterator itr = _models_list.begin();
  while(itr != _models_list.end())
  {
    QModelIndex foundModelIndex = (*itr)->findModelIndex(digest);
    if(foundModelIndex.isValid())
    {
      src_msg.first = *itr;
      src_msg.second = foundModelIndex;
      return src_msg;
    }
    else
      itr++;
  }

  return src_msg;
}

