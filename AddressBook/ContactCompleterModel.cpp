#include "ContactCompleterModel.hpp"

#include "AddressBookModel.hpp"
#include "Contact.hpp"

TContactCompletionModel::TContactCompletionModel(AddressBookModel* parent) :
  QAbstractListModel(parent), _dataSource(parent)
{
  setFiltredModel();
}

const Contact& TContactCompletionModel::getContact(const QModelIndex& index) const
{
  QModelIndex index_filtred = _filtred_data_source->index(index.row(), index.column());
  QModelIndex mapped_index = _filtred_data_source->mapToSource(index_filtred);
  return _dataSource->getContact(mapped_index);
}

QString TContactCompletionModel::getDisplayText(const Contact& contact) const
  {
  return QString::fromStdString(contact.get_display_name());
  }

int TContactCompletionModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
  {
  /// This model doesn't support parent-child relationships
  if(parent.isValid())
    return 0;
  /// Get number of contacts directly from address book model.
  //return _dataSource->rowCount(parent);
  return _filtred_data_source->rowCount(parent);
  }

QVariant TContactCompletionModel::data(const QModelIndex& index, int role) const
  {
  int row = index.row();
  if(index.isValid() && row >= 0 && row < rowCount())
    {
    switch(role)
      {
      case Qt::DisplayRole:
      case Qt::EditRole:
        {
        const Contact& requestedContact = getContact(index);
        QString text = getDisplayText(requestedContact);
        return QVariant(text);
        }
      case Qt::UserRole:
        return QVariant(row);
      default:
        /// Ignore other roles
        ;
      }
    }

  return QVariant();
  }

void TContactCompletionModel::setFiltredModel()
{
  if(_dataSource)
  {
    _filtred_data_source = new FilterBlockedModel(this);
    _filtred_data_source->setSourceModel(_dataSource);
    _filtred_data_source->setDynamicSortFilter(true);

    _filtred_data_source->setFilterBlocked(true);
  }
}

