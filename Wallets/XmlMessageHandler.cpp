#include "XmlMessageHandler.hpp"

XmlMessageHandler::XmlMessageHandler() :
QAbstractMessageHandler(0)
{
}

QString XmlMessageHandler::statusMessage() const
{
  return _description;
}

int XmlMessageHandler::line() const
{
  return _sourceLocation.line();
}

void XmlMessageHandler::handleMessage(QtMsgType type, const QString &description,
    const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  Q_UNUSED(type);
  Q_UNUSED(identifier);

  _messageType = type;
  _description = description;
  _sourceLocation = sourceLocation;
}

