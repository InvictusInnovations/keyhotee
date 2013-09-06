#pragma once
#include <QWizard>
#include <bts/application.hpp>

namespace Ui {
    class IntroPage;
    class ProfileEditPage;
} // namespace Ui
class ProfileWizard : public QWizard
{
   public:
     ProfileWizard( QWidget* parent, const bts::application_ptr& app );
     ~ProfileWizard();

     enum Pages
     { 
        Page_Intro,
        Page_Profile,
        Page_FirstNym
     };

   private:
     Ui::IntroPage*        _profile_intro_ui;
     Ui::ProfileEditPage*  _profile_edit_ui;

};
