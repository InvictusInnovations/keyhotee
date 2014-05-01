#include "ProfileWizard.hpp"

#include "KeyhoteeApplication.hpp"

#include "qtreusable/AutoUpdateProgressBar.hpp"

#include "ui_ProfileEditPage.h"
#include "ui_ProfileIntroPage.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QSettings>
//#include <ui_ProfileNymPage.h>

#include <fc/string.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <bts/addressbook/addressbook.hpp>
#include <iostream>

#if 0
class NymPage : public QWizardPage
{
public:
  NymPage(QWidget* parent)
    : QWizardPage(parent),
    _complete(false)
  {
       setTitle(tr("Create your Keyhotee ID") );
       _profile_nym_ui.setupUi(this);
   
       connect(_profile_nym_ui.keyhotee_id, &QLineEdit::textEdited, this, &NymPage::validateId);
       connect(_profile_nym_ui.avatar_button, &QPushButton::clicked, this, &NymPage::iconSearch);
  }

  virtual bool isComplete() const
  {
    return _complete;
  }

  /** starts a lookup timer that will send a query one second
   * after the most recent edit unless another character is
   * entered.
   */
  void validateId(const QString& id)
  {
    _complete = false;
    completeChanged();
    _last_validate = fc::time_point::now();
    _profile_nym_ui.id_warning->setText(tr("Checking availability of ID...") );
    fc::async( [ = ](){
                 fc::usleep(fc::microseconds(500 * 1000) );
                 if (fc::time_point::now() > (_last_validate + fc::microseconds(500 * 1000)) )
                   lookupId();
               }
               );
  }

  void lookupId()
  {
    try
    {
      auto current_id = _profile_nym_ui.keyhotee_id->text().toStdString();
      auto opt_name_record = bts::application::instance()->lookup_name(current_id);
      if (opt_name_record)
      {
        _profile_nym_ui.id_warning->setText(tr("This ID has been taken by another user") );
      }
      else
      {
        _profile_nym_ui.id_warning->setText(tr("This ID is available!") );
        _complete = true;
        completeChanged();
      }
    }
    catch (const fc::exception& e)
    {
      _profile_nym_ui.id_warning->setText(e.to_string().c_str() );
    }
  }

  void iconSearch()
  {
    auto writableLocation = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), writableLocation, tr("Image Files (*.png *.jpg *.bmp)"));
    _profile_nym_ui.id_warning->setText(fileName);                 // REVISIT!!!! testing: display filename.
  }

  fc::time_point _last_validate;
  bool           _complete;
  Ui::NymPage    _profile_nym_ui;
};
#endif

class ProfileEditPage : public QWizardPage
{
public:
  ProfileEditPage(QWidget* parent)
    : QWizardPage(parent)
  {
    setTitle(tr("Create your Keyhotee Profile") );
    ui.setupUi(this);
    connect(ui.generaterandom, &QPushButton::clicked, this, &ProfileEditPage::generateSeed);
    connect(ui.first_name, &QLineEdit::textChanged, this, &ProfileEditPage::firstNameEdited);
    connect(ui.brainkey, &QLineEdit::textChanged, this, &ProfileEditPage::brainKeyEdited);
    connect(ui.local_password1, &QLineEdit::textEdited, this, &ProfileEditPage::loginPasswordEdited);
    connect(ui.local_password2, &QLineEdit::textEdited, this, &ProfileEditPage::loginPasswordCheckEdited);
  }

  void generateSeed()
  {
    auto private_key = fc::ecc::private_key::generate();
    ui.brainkey->setText(std::string(private_key.get_secret()).c_str() );
    completeChanged();
  }

  void firstNameEdited(const QString& first_name)
  {
    completeChanged();
  }

  void brainKeyEdited(const QString& brain_key)
  {
    if(trimmedBrainkey().size() < 32)
      ui.brainkey_warning->setText(tr("Your Brain Key must be at least 32 characters") );
    else
      ui.brainkey_warning->setText(QString() );
    completeChanged();
  }

  void loginPasswordEdited(const QString& password)
  {
    ui.local_password2->setText(QString() );
    if (password.size() < 8)
    {
      ui.password_warning->setText(tr("Password must be at least 8 characters") );
      ui.local_password2->setEnabled(false);
    }
    else
    {
      ui.password_warning->setText(tr("Password do not match") );
      ui.local_password2->setEnabled(true);
    }
    completeChanged();
  }

  void loginPasswordCheckEdited(const QString& password)
  {
    if (ui.local_password1->text() == password)
      // TODO: check that password isn't on the commonly used
      ui.password_warning->setText(QString() );
    else
      ui.password_warning->setText(tr("Passwords do not match") );
    completeChanged();
  }

  QString trimmedFirstName() const
  {
    return ui.first_name->text().trimmed();
  }

  QString trimmedMiddleName() const
  {
    return ui.middle_name->text().trimmed();
  }

  QString trimmedLastName() const
  {
    return ui.last_name->text().trimmed();
  }
  
  QString trimmedBrainkey() const
  {
    return ui.brainkey->text().trimmed();
  }

  virtual bool isComplete() const
  {
    ui.first_name->setStyleSheet("");
    ui.brainkey->setStyleSheet("");
    ui.local_password1->setStyleSheet("");
    ui.local_password2->setStyleSheet("");

    if(trimmedFirstName().isEmpty())
    {
      ui.first_name->setStyleSheet("border: 1px solid red");
      return false;
    }
    if(trimmedBrainkey().size() < 32)
    {
      ui.brainkey->setStyleSheet("border: 1px solid red");
      return false;
    }
    if(ui.local_password1->text().size() < 8)
    {
      ui.local_password1->setStyleSheet("border: 1px solid red");
      return false;
    }
    if(ui.local_password1->text() != ui.local_password2->text())
    {
      ui.local_password2->setStyleSheet("border: 1px solid red");
      return false;
    }
    return true;
  }

  Ui::ProfileEditPage ui;
};

ProfileWizard::ProfileWizard(TKeyhoteeApplication& mainApp) :
  QWizard(nullptr),
  _mainApp(mainApp)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setOption(HaveHelpButton, true);

  QWizardPage* intro_page = new QWizardPage(this);
  intro_page->setTitle(tr("Welcome to Keyhotee") );
  _profile_intro_ui = new Ui::IntroPage();
  _profile_intro_ui->setupUi(intro_page);

#ifdef Q_OS_MAC
  _profile_intro_ui->introImage->setPixmap(QPixmap(QString::fromUtf8(":/images/keyhotee.icns")));
#else
  _profile_intro_ui->introImage->setPixmap(QPixmap(QString::fromUtf8(":/images/keyhotee.png")));
#endif

  _profile_intro_ui->gridLayout->addWidget(_profile_intro_ui->introImage, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignBottom);

//  _nym_page = new NymPage(this);
  _profile_edit = new ProfileEditPage(this);

  connect(this, &ProfileWizard::helpRequested, this, &ProfileWizard::showHelp);
  connect(this, &ProfileWizard::accepted, this, &ProfileWizard::createProfile);

  setPage(Page_Intro, intro_page);
  setPage(Page_Profile, _profile_edit);
//  setPage(Page_FirstNym, _nym_page);

  setStartId(Page_Intro);

   #ifndef Q_WS_MAC
  setWizardStyle(ModernStyle);
   #else
  setWizardStyle(MacStyle);
   #endif
}

ProfileWizard::~ProfileWizard()
{
  if (!_profile_edit->isComplete() )
    _mainApp.quit();
}

void ProfileWizard::showHelp()
{
  // TODO: open up the help browser and direct it to this page.
}

void ProfileWizard::createProfile()
{
  if (_profile_edit->isComplete() )
  {
    std::cerr << "createProfile()\n";
    ilog( "." );
    bts::profile_config conf;
    QString fName = _profile_edit->trimmedFirstName();
    QString mName = _profile_edit->trimmedMiddleName();
    QString lName = _profile_edit->trimmedLastName();
    conf.firstname  = fName.toStdString();
    conf.middlename = mName.toStdString();
    conf.lastname   = lName.toStdString();

    conf.brainkey   = _profile_edit->trimmedBrainkey().toStdString();

    std::string password = _profile_edit->ui.local_password1->text().toStdString();

    assert(conf.firstname.empty() == false && "Broken update command ui code");

    //set profile name from first and last name
    std::wstring profileName = fName.toStdWString();
    if(mName.isEmpty() == false)
      {
      profileName += L" ";
      profileName += mName.toStdWString();
      }

    if(lName.isEmpty() == false)
      {
      profileName += L" ";
      profileName += lName.toStdWString();
      }

    const unsigned int progressWidth = 640;
    const unsigned int progressHeight = 20;
    const unsigned int progressMax = 1000;
    int x=(qApp->desktop()->width() - progressWidth)/2;
    int y=(qApp->desktop()->height() - progressHeight)/2;

    TAutoUpdateProgressBar* progress = TAutoUpdateProgressBar::create(QRect(x, y, progressWidth,
      progressHeight), tr("Creating Profile"), progressMax);

    /** \warning Copy mainApp pointer to local variable, since lambda needs its copy
        this object is destroyed while executing create_profile action.
    */
    TKeyhoteeApplication* mainApp = &_mainApp;

    auto app = bts::application::instance();

    progress->doTask(
      [=]() 
      {
        auto profile = app->create_profile(profileName, conf, password, 
          [=]( double p )
          {
            progress->updateValue(progressMax*p);
          });
      },
      [=]() 
      {
        mainApp->setLoadedProfileName(QString::fromStdWString(profileName));
        QSettings settings("Invictus Innovations", "Keyhotee");
        settings.setValue("last_profile", QString::fromStdWString(profileName));
        mainApp->displayMainWindow();
      },
        [=](QString e)
      {
      std::cerr << "Error creating profile\n";
      if(e.isEmpty())
        {
        QMessageBox::warning(nullptr, tr("Profile wizard"),
          tr("Cannot create profile - some unknown error occurred"));
        }
      else
        {
        QMessageBox::warning(nullptr, tr("Profile wizard"), tr("Cannot create profile: ") + e);
        }
      }
    );
  }
  else
    {
    std::cerr << "createProfile: Incomplete profile\n";
    ilog( "createProfile: Incomplete profile" );
    }
}

