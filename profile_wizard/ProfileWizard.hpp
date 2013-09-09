#pragma once
#include <QWizard>
#include <bts/application.hpp>

namespace Ui {
    class IntroPage;
} // namespace Ui

class NymPage;
class ProfileEditPage;

class ProfileWizard : public QWizard
{
   public:
     ProfileWizard( QWidget* parent );
     ~ProfileWizard();

     enum Pages
     { 
        Page_Intro,
        Page_Profile,
        Page_FirstNym
     };

   public slots:
     void showHelp();
     void createProfile( int result );

   private:
     NymPage*              _nym_page;
     ProfileEditPage*      _profile_edit;
     Ui::IntroPage*        _profile_intro_ui;
};
