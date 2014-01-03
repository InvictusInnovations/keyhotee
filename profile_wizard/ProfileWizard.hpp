#pragma once
#include <QWizard>
#include <bts/application.hpp>

namespace Ui {
  class IntroPage;
  } // namespace Ui

class NymPage;
class ProfileEditPage;

class TKeyhoteeApplication;

class ProfileWizard : public QWizard
{
public:
  enum Pages
    {
    Page_Intro,
    Page_Profile,
    Page_FirstNym
    };

public slots:
  void showHelp();
  void createProfile();

private:
  /// Only main app is allowed to create this object.
  friend class TKeyhoteeApplication;
  ProfileWizard(TKeyhoteeApplication& mainApp);
  virtual ~ProfileWizard();

  /// Class attributes:
private:
  TKeyhoteeApplication& _mainApp;
  NymPage*              _nym_page;
  ProfileEditPage*      _profile_edit;
  Ui::IntroPage*        _profile_intro_ui;
};
