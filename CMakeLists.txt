set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )
find_package( Qt5Widgets )
find_package( Qt5WebKitWidgets )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Only build the Keyhotee gui when we find Qt
IF( NOT Qt5Widgets_DIR MATCHES Qt5Widgets_DIR-NOTFOUND )

    set( headers
         profile_wizard/ProfileWizard.hpp
       )

    qt5_wrap_ui( profile_wizard/UiProfileEditPage  profile_wizard/ProfileEditPage.ui )
    qt5_wrap_ui( profile_wizard/UiProfileIntroPage profile_wizard/ProfileIntroPage.ui )
    qt5_wrap_ui( profile_wizard/UiNymPage          profile_wizard/ProfileNymPage.ui )
    qt5_wrap_ui( UiLoginDialog                     LoginDialog.ui )

    set( sources  
         profile_wizard/ProfileEditPage.ui 
         profile_wizard/ProfileIntroPage.ui 
         profile_wizard/ProfileNymPage.ui 
         profile_wizard/ProfileWizard.cpp
         LoginDialog.ui 
         LoginDialog.cpp
         main.cpp )

    add_executable( keyhotee MACOSX_BUNDLE WIN32 ${sources} )
    target_link_libraries( keyhotee upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} Qt5::Widgets Qt5::WebKitWidgets)

ENDIF( NOT Qt5Widgets_DIR MATCHES Qt5Widgets_DIR-NOTFOUND )
