#ifndef FILEATTACHMENTWIDGET_H
#define FILEATTACHMENTWIDGET_H

//#include <bts/addressbook/contact.hpp>
#include <bts/application.hpp>

#include <QFileInfo>
#include <QWidget>

#include <list>
#include <utility>

class Contact;

namespace Ui {
class TFileAttachmentWidget;
}

/** Implements file attachment widget providing file attach GUI functionality to the email editor.
*/
class TFileAttachmentWidget : public QWidget
  {
  Q_OBJECT

  public:
    /// Data container to be filled with collected files being attached to the email.
    typedef std::vector<bts::bitchat::attachment> TAttachmentContainer;
    typedef std::list<QFileInfo>                  TFileInfoList;

    TFileAttachmentWidget(QWidget* parent, bool editMode = false);
    virtual ~TFileAttachmentWidget();

    /** Allows to load set of files (or contacts files with extension *.vcf) 
        attached to already existing email message (ie in Draft).
    */
    void LoadAttachedFiles(const TAttachmentContainer& attachedFiles);

    /** Retrieves attached files, reads them and puts their contents into specified container.
        Returns false if some of originally attached files is not readable or doesn't exists anymore.

        \param storage - final storage for the mail attachment. Cannot be null.
        \param failedFilesStorage - storage for the infos of files which have been failed to attach
                         (because they don't exist anymore or are not readable).
    */
    bool GetAttachedFiles(TAttachmentContainer* storage, TFileInfoList* failedFilesStorage) const;

    /// Returns true when widget allows to make any changes in its contents.
    bool AllowEdits() const
      {
      return EditMode;
      }

    void selectAllFiles();
    bool saveAttachments();
    bool hasAttachment();
    /// Add attachments files 
    void addFiles(const QStringList& files);
    /// Add contact to attachment list
    void addContactCard(const Contact& contact);
    
    /// Signal emitted when attachment list changes.
    Q_SIGNAL void attachmentListChanged();

  private:
    class AAttachmentItem;
    class TFileAttachmentItem;
    class TVirtualAttachmentItem;

    typedef bts::bitchat::attachment TPhysicalAttachment;
    /// Pair of scaled attachment size and its unit (KB, MB etc).
    typedef std::pair<double, const char*> TScaledSize;
    /// Helper container to store currently selected items.
    typedef std::vector<AAttachmentItem*>  TSelection;

    /// Simple enum to return attachment item Save operation status.
    enum TSaveStatus : int
      {
      READ_SOURCE_ERROR  = 1,
      WRITE_TARGET_ERROR,
      SUCCESS 
      };

    /// Allows to prebuild context menu specific to attachment table.
    void ConfigureContextMenu();
    /// Configures other properties of attachment table.
    void ConfigureAttachmentTable();
    void UpdateColumnHeaders();
    /// Allows to scale given size for well presentation purposes.
    TScaledSize ScaleAttachmentSize(unsigned long long size) const;
    bool FreezeAttachmentTable();
    void UnFreezeAttachmentTable(bool sortEnabled);
    /// Called when attachment item name has been changed.
    void OnAttachmentItemChanged();
    void AddAttachmentItems(AAttachmentItem* fileNameItem, AAttachmentItem* fileSizeItem);

    /// Allows to convert given scaled size to the string format.
    static QString toString(const TScaledSize& size);
    /// Helper function to implement attachment item saving.
    bool SaveAttachmentItem(const AAttachmentItem* iten, const QFileInfo& targetPath,
      bool checkForOverwrite);
    void RetrieveSelection(TSelection* storage) const;   
    /** Check validation of contact vCard format.
        Returns false if vCard format is not valid
        \param fileName - path to vCard data file
        \param item - current selected item on the attachment table
        \param contactData - returns contact vCard data
    */
    static bool isValidContactvCard(QString fileName, const AAttachmentItem& item, QByteArray* contactData);

  private slots:
    void onAddTriggered();
    void onDelTriggered();
    void onOpenTriggered();
    void onSaveTriggered();
    void onRenameTriggered();
    void onAttachementTableSelectionChanged();
    void onPasteTriggered();
    void onAddContactTriggered();
    void onImportContactTriggered();
    void onFindContactTriggered();
    void onClipboardChanged();    
    void onDropEvent(QStringList files);

  /// Class attributes:
  private:
    typedef std::list<AAttachmentItem*> TFileAttachmentList;

    Ui::TFileAttachmentWidget*        ui;
    QStringList                       SelectedFiles;
    /// Used to perform checks related to redundant files.
    TFileInfoList                     AttachmentIndex;
    /** Stores attachment info (file-name items stored in table widget) in order they were added
        to table.
    */
    TFileAttachmentList               AttachmentList;
    /// Total size of all file attachments.
    unsigned long long                TotalAttachmentSize;
    /// Simple flag determining which features can be available.
    bool                              EditMode;
    /// Holds data about contact which has been right clicked
    bts::addressbook::wallet_contact  _clickedContact;
  };

#endif // FILEATTACHMENTWIDGET_H

