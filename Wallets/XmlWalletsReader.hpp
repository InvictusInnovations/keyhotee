#pragma once

#include "WalletsGui.hpp"

class QFile;
class QWidget;
class QXmlStreamReader;

class XmlWalletsReader
{
public:
  XmlWalletsReader(QWidget* parent, QList<WalletsGui::Data>* data);
  virtual ~XmlWalletsReader();

private:
  /// fill _data wllets structure
  bool parseXML(const QString& fileName);
  /// @returns WalletsGui data from XML file
  WalletsGui::Data parseWallet(QXmlStreamReader& xml, const QString& fileName); 
  /// @returns empty string if attribute not found
  QString readAttribute(QXmlStreamReader& xml, QStringRef attributem, const QString& fileName);
  /// read <server> section from XML file and add data to 'wallet' structure
  void readServerSection(QXmlStreamReader& xml, const QString& fileName, WalletsGui::Data* wallet);

private:
  QWidget* _parent;
	QList<WalletsGui::Data>* _data;
};

