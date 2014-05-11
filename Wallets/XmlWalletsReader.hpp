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

private:
  QWidget* _parent;
	QList<WalletsGui::Data>* _data;
};

