#include "XmlWalletsReader.hpp"

#include <QIODevice>
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QXmlStreamReader>

XmlWalletsReader::XmlWalletsReader(QWidget* parent, QList<WalletsGui::Data>* data)
: _parent(parent), _data(data)
{
  QString xmlFileName = ":Wallets/DefaultWallets.xml";
  QFile file(xmlFileName);
  if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) 
  {   
    QMessageBox::critical(parent,
      QObject::tr("Wallets reading ..."), 
      QObject::tr("Error opening file: ") + xmlFileName);
  }

  QXmlStreamReader xml(&file);

  while (xml.atEnd() == false && 
        xml.hasError() == false)
  {
    /// Read next element
    QXmlStreamReader::TokenType token = xml.readNext();    

    const QString *name = xml.name().string();

    /// If token is just StartDocument, we'll go to next
    if (token == QXmlStreamReader::StartDocument)
    {
      continue;
    }

    /// If token is StartElement, we'll see if we can read it
    if (token == QXmlStreamReader::StartElement) 
    {
      if (xml.name() == "wallets")
      {
        /// check wallets version
        if (xml.attributes().value("version") == "1.0")
        {
          xml.readNextStartElement();
          continue;          
        }
        else
          xml.raiseError(QObject::tr("The file is not Wallets version 1.0."));
      }

      if (xml.name() == "wallet") 
      {
        /// read wallet attributes;
      }
    }
  }

  /* Error handling. */
  if (xml.hasError()) 
  {
    QMessageBox::critical(_parent,
      QObject::tr("Wallets reading ..."),
      xml.errorString()
      );
  }
  /* Removes any device() or data from the reader
  * and resets its internal state to the initial state. */
  xml.clear();
}

XmlWalletsReader::~XmlWalletsReader()
{
}