#include "InfoBar.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

void TInfoBar::showInfoBar(const QString& textToDisplay, const QString* acceptBtnText,
  QBoxLayout* parent, int index)
  {
  TInfoBar* bar = new TInfoBar(parent);
  bar->buildWidgets(textToDisplay, acceptBtnText, parent, index);
  }

inline
TInfoBar::TInfoBar(QObject* objParent) : QObject(objParent),
  _parent(nullptr), _mainContainer(nullptr) {}

TInfoBar::~TInfoBar() {}

void TInfoBar::buildWidgets(const QString& textToDisplay, const QString* acceptBtnText,
  QBoxLayout* parent, int index)
  {
  _mainContainer = new QFrame;

  QPalette pal = _mainContainer->palette();
  pal.setColor(QPalette::Window, QColor(255, 255, 225));
  pal.setColor(QPalette::WindowText, Qt::black);

  _mainContainer->setPalette(pal);
  _mainContainer->setFrameStyle(QFrame::Panel | QFrame::Raised);
  _mainContainer->setLineWidth(1);
  _mainContainer->setAutoFillBackground(true);

  QHBoxLayout* hBox = new QHBoxLayout(_mainContainer);
  hBox->setMargin(2);

  QLabel* infoWidgetLabel = new QLabel(textToDisplay);
  infoWidgetLabel->setWordWrap(true);
  hBox->addWidget(infoWidgetLabel);

  if(acceptBtnText != nullptr)
    {
    QToolButton* infoWidgetButton = new QToolButton;
    infoWidgetButton->setText(*acceptBtnText);
    connect(infoWidgetButton, SIGNAL(clicked()), this, SLOT(infoBarAccepted()));

    hBox->addWidget(infoWidgetButton);
    }

  QToolButton* infoWidgetCloseButton = new QToolButton;

  connect(infoWidgetCloseButton, SIGNAL(clicked()), SLOT(cancelBtnClicked()));

  infoWidgetCloseButton->setAutoRaise(true);
  infoWidgetCloseButton->setIcon(QIcon(QLatin1String(":/images/close_button.png")));
  infoWidgetCloseButton->setToolTip(tr("Close"));
  hBox->addWidget(infoWidgetCloseButton);

  _parent->insertWidget(index, _mainContainer);
  }

void TInfoBar::cancelBtnClicked()
  {
  _parent->removeWidget(_mainContainer);
  }

