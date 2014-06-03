#ifndef __CONTACTCOMPLETERMODEL_HPP
#define __CONTACTCOMPLETERMODEL_HPP

#include <QAbstractListModel>

class AddressBookModel;
class FilterBlockedModel;
class Contact;

/** Dedicated data model for contact completer directly operating on AddressBookModel.
*/
class TContactCompletionModel : public QAbstractListModel
  {
  public:
    explicit TContactCompletionModel(AddressBookModel* parent);
    virtual ~TContactCompletionModel() {}

    /// Allows to retrieve contact representation pointed by given index.
    const Contact& getContact(const QModelIndex& index) const;
    /// Allows to retrieve display text for given contact (in form it is presented in completer).
    QString getDisplayText(const Contact& contact) const;

  /// QAbstractItemModel reimplementation:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data( const QModelIndex& index, int role ) const override;

  private:
    void setFiltredModel();

  /// Class atrributes:
  private:
    AddressBookModel*   _dataSource;
    FilterBlockedModel* _filtred_data_source;
  };


#endif /// __CONTACTCOMPLETERMODEL_HPP

