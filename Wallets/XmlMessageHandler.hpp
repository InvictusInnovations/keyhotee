#pragma once

#include <QAbstractMessageHandler>

class XmlMessageHandler : public QAbstractMessageHandler
{
public:
  XmlMessageHandler();
  virtual ~XmlMessageHandler(){};

  QString statusMessage() const;
  int line() const;

protected:
  virtual void handleMessage(QtMsgType type, const QString &description,
    const QUrl &identifier, const QSourceLocation &sourceLocation);

private:
  QtMsgType _messageType;
  QString _description;
  QSourceLocation _sourceLocation;
};

