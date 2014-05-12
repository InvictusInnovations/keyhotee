#include "WalletsGui.hpp"
#include "XmlWalletsReader.hpp"
#include <QWidget>

WalletsGui::WalletsGui(QWidget* parent)
{
  read(parent);
}


void WalletsGui::read(QWidget* parent)
{
  XmlWalletsReader reader(parent, &_data);
}

const QList	<WalletsGui::Data>& WalletsGui::getData() const
{
  return _data;
}