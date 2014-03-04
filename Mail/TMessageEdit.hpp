#ifndef TMESSAGEEDIT_HPP
#define TMESSAGEEDIT_HPP

#include <QTextEdit>

namespace bts
{
namespace bitchat
{
struct attachment;

} ///namespace bitchat
} /// namespace bts

class TMessageEdit : public QTextEdit
{
    Q_OBJECT

public:

  typedef std::vector<bts::bitchat::attachment> Attachments;

  explicit TMessageEdit(QWidget *parent = 0);
  /** Loads mail message and images
      \param body - mail message body
      \param attachments - when attachments contain images display it.
  */
  void loadContents (const QString& body, const Attachments& attachments);

protected:
    virtual void insertFromMimeData(const QMimeData *source) override;

signals:
  void addAttachments(QStringList);

public slots:

};

#endif // TMESSAGEEDIT_HPP
