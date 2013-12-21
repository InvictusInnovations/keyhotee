#ifndef FILEATTACHMENTWIDGET_H
#define FILEATTACHMENTWIDGET_H

#include <bts/application.hpp>

#include <QFileInfo>
#include <QWidget>

#include <list>
#include <utility>

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

    TFileAttachmentWidget(QWidget* parent, bool editMode);
    virtual ~TFileAttachmentWidget();

    /** Allows to load set of files attached to already existing email message (ie in Draft).
    */
    void LoadAttachedFiles(const TAttachmentContainer& attachedFiles);

    /** Retrieves attached files, reads them and puts their contents into specified container.
        Returns false if some of originally attached files is not readable or doesn't exists anymore.

        \param storage - final storage for the mail attachment. Cannot be null.
        \param failedFilesStorage - storage for the infos of files which have been failed to attach
                         (because they don't exist anymore or are not readable).
    */
    bool GetAttachedFiles(TAttachmentContainer* storage, TFileInfoList* failedFilesStorage) const;
    
    /// Signal emitted when attachment list changes.
    Q_SIGNAL void attachmentListChanged();

  private:
    class TFileAttachmentItem;
    typedef bts::bitchat::attachment TPhysicalAttachment;

    /// Pair of scaled attachment size and its unit (KB, MB etc).
    typedef std::pair<double, const char*> TScaledSize;

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

    /** Physically attaches the file attachment to specified email storage.
        \param item    - attachment item to be processed.
        \param storage - final attachment storage,
        \param failedFilesStorage - storage for file infos for which the attachment process failed.
    */
    void AttachFile(const TFileAttachmentItem& item, TAttachmentContainer* storage,
      TFileInfoList* failedFilesStorage) const;

    /// Allows to convert given scaled size to the string format.
    static QString toString(const TScaledSize& size);


  private slots:
    void onAddTriggered();
    void onDelTriggered();
    void onSaveTriggered();
    void onRenameTriggered();
    void onAttachementTableSelectionChanged();

  /// Class attributes:
  private:
    typedef std::list<TFileAttachmentItem*>  TFileAttachmentList;

    Ui::TFileAttachmentWidget* ui;
    QStringList                SelectedFiles;
    /// Used to perform checks related to redundant files.
    TFileInfoList              AttachmentIndex;
    /** Stores attachment info (file-name items stored in table widget) in order they were added
        to table.
    */
    TFileAttachmentList        AttachmentList;
    /// Total size of all file attachments.
    unsigned long long         TotalAttachmentSize;
    /// Simple flag determining which features can be available.
    bool                       EditMode;
  };

#endif // FILEATTACHMENTWIDGET_H

