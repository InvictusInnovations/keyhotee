#pragma once

#include "WalletsGui.hpp"

class QWidget;

class XmlWalletsReader
{
public:
  XmlWalletsReader(QWidget* parent, QList<WalletsGui::Data>* data);
  virtual ~XmlWalletsReader();

private:
  QWidget* _parent;
	QList<WalletsGui::Data>* _data;
};

