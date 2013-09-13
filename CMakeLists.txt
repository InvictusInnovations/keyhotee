set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )
find_package( Qt5Widgets )
find_package( Qt5WebKitWidgets )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Only build the Keyhotee gui when we find Qt
IF( NOT Qt5Widgets_DIR MATCHES Qt5Widgets_DIR-NOTFOUND )
   IF( APPLE )
      SET_SOURCE_FILES_PROPERTIES(
                                  "images/shield.icns"
                                  PROPERTIES
                                  MACOSX_PACKAGE_LOCATION Resources
                                  )
      SET( MACOSX_BUNDLE_BUNDLE_NAME "Keyhotee" )
      SET( MACOSX_BUNDLE_GUI_IDENTIFIER "com.invictus-innovations.Keyhotee" ) 
      set( MACOSX_BUNDLE_ICON_FILE shield.icns )
      SET( MACOSX_BUNDLE_INFO_STRING "Keyhotee - version 1.0" ) 
      SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0" ) 
      SET( MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0" ) 
      SET( MACOSX_BUNDLE_BUNDLE_VERSION "1.0" ) 
      SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2013 Invictus Innovations" ) 
  
    ENDIF( APPLE )

    set( headers
         profile_wizard/ProfileWizard.hpp
       )

    qt5_wrap_ui( profile_wizard/UiProfileEditPage  profile_wizard/ProfileEditPage.ui )
    qt5_wrap_ui( profile_wizard/UiProfileIntroPage profile_wizard/ProfileIntroPage.ui )
    qt5_wrap_ui( profile_wizard/UiNymPage          profile_wizard/ProfileNymPage.ui )
    qt5_wrap_ui( UiLoginDialog                     LoginDialog.ui )
    qt5_wrap_ui( UiKeyhoteeMainWindow              KeyhoteeMainWindow.ui )
    qt5_wrap_ui( UiContactsTable                   AddressBook/ContactsTable.ui )
    qt5_wrap_ui( UiContactView                     AddressBook/ContactView.ui )
    QT5_ADD_RESOURCES( KeyhoteeQRC  Keyhotee.qrc )

    set( library_sources
         AddressBook/AddressBookModel.hpp
         AddressBook/AddressBookModel.cpp )

    set( sources  
         Keyhotee.qrc 
         images/shield.icns 
         qrc_Keyhotee.cpp

         profile_wizard/ProfileEditPage.ui 
         profile_wizard/ProfileIntroPage.ui 
         profile_wizard/ProfileNymPage.ui 
         profile_wizard/ProfileWizard.cpp

         AddressBook/ContactsTable.ui
         AddressBook/ContactsTable.hpp
         AddressBook/ContactsTable.cpp
         AddressBook/ContactView.ui
         AddressBook/ContactView.hpp
         AddressBook/ContactView.cpp

         LoginDialog.ui 
         LoginDialog.cpp

         KeyhoteeMainWindow.ui 
         KeyhoteeMainWindow.cpp 
         main.cpp )










    add_library( keyhotee_library ${library_sources} )
    target_link_libraries( keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} Qt5::Widgets Qt5::WebKitWidgets ${QtMacExtras} ${APPKIT_LIBRARY})

    add_executable( Keyhotee WIN32 MACOSX_BUNDLE ${sources} )
    target_link_libraries( Keyhotee keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} Qt5::Widgets Qt5::WebKitWidgets ${QtMacExtras} ${APPKIT_LIBRARY})


ENDIF( NOT Qt5Widgets_DIR MATCHES Qt5Widgets_DIR-NOTFOUND )
