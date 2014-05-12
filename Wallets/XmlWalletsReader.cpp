#include "XmlWalletsReader.hpp"

#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QXmlStreamReader>

XmlWalletsReader::XmlWalletsReader(QWidget* parent, QList<WalletsGui::Data>* data)
: _parent(parent), _data(data)
{
  bool xmlFileError = false;
  QString xmlDefault = ":Wallets/DefaultWallets.xml";
  QString xmlFileName = "WalletsGui.xml";  

  QFileInfo fileInfo(xmlFileName);
  /// Display absolute file path
  xmlFileName = fileInfo.absoluteFilePath();

  if (QFile::exists(xmlFileName) == false)
  {
    /// copy default xml file to Keyhotee.exe path
    if (QFile::copy(xmlDefault, xmlFileName) == false)
    {
      QMessageBox::critical(_parent, QObject::tr("Wallets reading ..."),
        QObject::tr("Can't copy file from ""%1"" \nto ""%2"" ").arg(xmlDefault, xmlFileName));
      xmlFileError = true;
    }
  }
 

  if (xmlFileError == false)
  {
    if (parseXML(xmlFileName) == false)
    {
      /// error parsing user wallets xml file
      xmlFileError = true;
    }
  }

  /// if any error exists,  parse default xml file from resource
  if (xmlFileError == true)
  {
    Q_ASSERT (parseXML(xmlDefault) == true);
  }
}

XmlWalletsReader::~XmlWalletsReader()
{
}


bool XmlWalletsReader::parseXML(const QString& fileName)
{
  _data->clear();

  QFile file(fileName);   

  if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
  {
    QMessageBox::critical(_parent,
      QObject::tr("Wallets reading ..."),
      QObject::tr("Error opening file: %1").arg(fileName));
  }

  QXmlStreamReader xml(&file);

  while (xml.atEnd() == false &&
    xml.hasError() == false)
  {
    /// Read next element
    QXmlStreamReader::TokenType token = xml.readNext();

    /// If token is StartElement, we'll see if we can read it
    if (token == QXmlStreamReader::StartElement)
    {
      if (xml.name() == "wallets")
      {
        if (xml.attributes().value("version") != "1.0")
        {
          /// wrong version
          xml.raiseError(
            QObject::tr("The file: %1 \nis not wallets version 1.0.").arg(fileName));
        }
      }
      else if (xml.name() == "wallet")
      {
        _data->push_back(parseWallet(xml, fileName));
      }
    }
  }

  /// Error handling
  if (xml.hasError())
  {    
    QString lineNumber = 
      QObject::tr("\n(Line number: %1)").arg(QString::number(xml.lineNumber()));
    QMessageBox::critical(_parent, QObject::tr("Wallets reading ..."),
      xml.errorString() + lineNumber);
    return false;
  }

  return true;
}

WalletsGui::Data XmlWalletsReader::parseWallet(QXmlStreamReader& xml, const QString& fileName)
{  
  WalletsGui::Data wallet;

  /// Get the attributes for wallet
  QXmlStreamAttributes attributes = xml.attributes();
  if (attributes.hasAttribute("name")) 
  {
    wallet.name = attributes.value("name").toString();
  }
  else
  {
    /// Attribute 'name' not found in the file:
    xml.raiseError(QObject::tr("Attribute ""name"" not found in the file: %1\n").arg(fileName));
    return wallet;
  }
  /// Next element...
  xml.readNext();

  /// continue the loop until we hit an EndElement named wallet
  while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
           xml.name() == "wallet")) 
  {
    if (xml.atEnd() == true || xml.hasError() == true)
      return wallet;

    if (xml.tokenType() == QXmlStreamReader::StartElement) 
    {
      if (xml.name() == "url") 
      {
        /// read attribute from element
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::Characters)
        {
          wallet.url = xml.text().toString();
        }
        else
        {
          xml.raiseError(QObject::tr("Attribute ""url"" not found in the file: %1\n").arg(fileName));
          return wallet;
        }
      }
      else if (xml.name() == "icon")
      {
        /// read attribute from element
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::Characters)
        {
          wallet.iconPath = xml.text().toString();
        }
        else
        {
          xml.raiseError(QObject::tr("Attribute ""icon"" not found in the file: %1\n").arg(fileName));
          return wallet;
        }
      }
    }

    xml.readNext();
  }
  return wallet;
}