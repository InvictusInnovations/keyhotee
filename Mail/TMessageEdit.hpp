#ifndef TMESSAGEEDIT_HPP
#define TMESSAGEEDIT_HPP

#include <bts/bitchat/bitchat_private_message.hpp>

#include <QTextBrowser>

#include <vector>

class IBlockerDelegate;

/** QTextBrowser subclass dedicated to notify client code about pasting/dropping some file attachment
    items into document body. Then attachmentAdded signal is emitted, pasted contents is ignored and
    client can decide where save received content to.
    This class also will suppport feature controlling automatic image loading pointed by external
    urls to increase security.
*/
class TMessageEdit : public QTextBrowser
{
Q_OBJECT
public:
  typedef std::vector<bts::bitchat::attachment> TAttachmentContainer;

  TMessageEdit(QWidget *parent = 0);
  virtual ~TMessageEdit() {}

  void initial(IBlockerDelegate* blocker);

  /** Loads mail message and images
      \param body        - mail message body
      \param attachments - checks attachments and if it contains images displays it by inlining
                           them at the end of document.
      \param anyBlockedImage - returns true if any remote image is blocked
  */
  void loadContents (const QString& body, const TAttachmentContainer& attachments);

  /// Allows to load blocked images (if any).
  void loadBlockedImages();

signals:
  /// Signal emitted when some attachement item is pasted/dropped into document.
  void attachmentAdded(const QStringList& attachementItems);

protected:
/// QTextBrowser class reimplementation.
  /// Reimplemented to support attachment item pasting. Then attachmentAdded signal is emitted.
  virtual void insertFromMimeData(const QMimeData *source) override;
  /// Reimplemented to support control over inline images/http links loading to improve security.
  virtual QVariant loadResource(int type, const QUrl& url) override;

private:
  /// Get unique local path to remote an image
  QString getCachedImagePath(const QString &remoteUrl);
  /// DownloadImage remote an image form a url
  bool downloadImage(const QString &remoteUrl);
  QString getFileExtension(const QString &fileName);

/// Class attributes:
private:
  bool                  _imageLoadAllowed;
  bool                  _anyBlockedImage;
  IBlockerDelegate*     _blocker;
};

#endif // TMESSAGEEDIT_HPP
