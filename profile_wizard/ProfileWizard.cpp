#include "ProfileWizard.hpp"
#include <ui_ProfileEditPage.h>
#include <ui_ProfileIntroPage.h>

ProfileWizard::ProfileWizard( QWidget* parent, const bts::application_ptr& app )
:QWizard(parent)
{
   QWizardPage* intro_page = new QWizardPage(this);
   _profile_intro_ui = new Ui::IntroPage();
   _profile_intro_ui->setupUi(intro_page);

   QWizardPage* profile_page = new QWizardPage(this);
   _profile_edit_ui  = new Ui::ProfileEditPage();
   _profile_edit_ui->setupUi(profile_page);

   setPage( Page_Intro, intro_page );
   setPage( Page_Profile, profile_page );
   setPage( Page_FirstNym, new QWizardPage() );

   setStartId( Page_Intro );

   #ifndef Q_WS_MAC
      setWizardStyle( ModernStyle );
   #else
      setWizardStyle( MacStyle );
   #endif
}

ProfileWizard::~ProfileWizard()
{
}
