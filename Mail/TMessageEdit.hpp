#ifndef TMESSAGEEDIT_HPP
#define TMESSAGEEDIT_HPP

#include <QTextBrowser>

/** QTextBrowser subclass dedicated to notify client code about pasting/dropping some file attachment
    items into document body. Then attachmentAdded is emitted, pasted contents is ignored and client
    can decide save pasted content to.
*/
class TMessageEdit : public QTextBrowser
{
Q_OBJECT
public:
  TMessageEdit(QWidget *parent = 0);
  virtual ~TMessageEdit() {}

signals:
  /// Signal emitted when some attachement item is pasted into document.
  void attachmentAdded(const QStringList& attachementItems);

protected:
  /// Reimplemented to support attachment item pasting. Then attachmentAdded signal is emitted.
  virtual void insertFromMimeData(const QMimeData *source) override;
  virtual QVariant loadResource(int type, const QUrl& url) override;
};

#endif // TMESSAGEEDIT_HPP
