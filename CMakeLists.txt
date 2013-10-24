project( keyhotee )
cmake_minimum_required( VERSION 2.8.11 )

set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )

IF( WIN32 )
#  set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake;C:/gh/qt5.1.1/qtwebkit/WebKitBuild/Release/lib/cmake/Qt5WebKitWidgets;"  )
  set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake"  )
  message(CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH})
ENDIF(WIN32)

find_package( Qt5Widgets )
include_directories(${Qt5Widgets_INCLUDE_DIRS})

if (WIN32)
  MESSAGE(platform= "Compiling on WIN32")
  set(QTDIR $ENV{QTDIR})
  message(qtdir=${QTDIR})
  set(QTWEBKITDIR ../qt5.1.1/qtwebkit)
  include_directories(${QTDIR}/include)
  set(QtPrintSupport_include $ENV{QTDIR}/include/QtPrintSupport)
  message(include+=${QtPrintSupport_include})
  include_directories(${QtPrintSupport_include})
  link_directories(${QTDIR}/lib)
  
  include_directories(${QTWEBKITDIR}/include)
  include_directories(${QTWEBKITDIR}/include/QtWebKitWidgets)
  link_directories(${QTWEBKITDIR}/WebKitBuild/Release/lib)
else(WIN32)
  find_package( Qt5WebKitWidgets )
endif(WIN32)

#Note: bitshares repo should be checked out under keyhotee dir or path here should be changed
set(BITSHARES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BitShares)
message(${BITSHARES_DIR})
include_directories(${BITSHARES_DIR}/include)
include_directories(${BITSHARES_DIR}/fc/include)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


IF( WIN32 )
  ADD_DEFINITIONS( -DWIN32 )
  ADD_DEFINITIONS( -DNOMINMAX )
  #ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  #ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB -DBOOST_THREAD_BUILD_LIB)
  ADD_DEFINITIONS( -DWIN32)
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -DLEVELDB_PLATFORM_WINDOWS )
  # Activate C++ exception handling
  IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")    
  ENDIF()
  #boost
  SET( Boost_INCLUDE_DIR $ENV{BOOST_ROOT} )
  #openssl
  #set( OPENSSL_ROOT_DIR "${BITSHARES_DIR}/vendor/openssl" )
  #set( LIB_EAY_RELEASE "${BITSHARES_DIR}/vendor/openssl/out32dll/libeay32.lib" )
  #set( SSL_EAY_RELEASE "${BITSHARES_DIR}/vendor/openssl/out32dll/ssleay32.lib" )
  #include_directories( "${BITSHARES_DIR}/vendor/openssl/include" )

  include_directories( ${BITSHARES_DIR}/vendor/leveldb-win/include )
ELSE(WIN32)
  include_directories( ${BITSHARES_DIR}/vendor/leveldb-1.12.0/include )
  include_directories( ${BITSHARES_DIR}/vendor/qtmacextras/include )

  MESSAGE(status "Compiling on UNIX")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs -fmax-errors=3 -msse4.2" ) 
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs -fmax-errors=3 -msse4.2" )
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs" ) 
  #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs" )
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context coroutine) 

 IF( NOT APPLE )
      SET( pthread pthread )
 ENDIF( NOT APPLE )
ENDIF(WIN32)

FIND_PACKAGE( OpenSSL )
include_directories( ${OPENSSL_INCLUDE_DIR} )
include_directories( ${Boost_INCLUDE_DIR} )
SET( ALL_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} ${SSL_EAY_RELEASE} ${LIB_EAY_RELEASE})

add_subdirectory(bitshares)

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
qt5_wrap_ui( UiMailbox                         Mail/Mailbox.ui )
qt5_wrap_ui( UiMailViewer                      Mail/MailViewer.ui )

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

        Mail/MailEditor.hpp
        Mail/MailEditor.cpp

        Mail/MailboxModel.hpp
        Mail/MailboxModel.cpp

        Mail/Mailbox.ui
        Mail/Mailbox.hpp
        Mail/Mailbox.cpp

        Mail/MailViewer.ui
        Mail/MailViewer.hpp
        Mail/MailViewer.cpp

        LoginDialog.ui 
        LoginDialog.cpp

        ContactListEdit.hpp
        ContactListEdit.cpp

        KeyhoteeMainWindow.ui 
        KeyhoteeMainWindow.cpp 
        main.cpp )




add_library( keyhotee_library ${library_sources} )
message(BOOST_LIBRARIES=${BOOST_LIBRARIES})
target_link_libraries( keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} Qt5::Widgets Qt5::WebKitWidgets ${QtMacExtras} ${APPKIT_LIBRARY})
 

add_executable( Keyhotee WIN32 MACOSX_BUNDLE ${sources} )
target_link_libraries( Keyhotee keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} Qt5::Widgets  Qt5::WebKitWidgets Qt5::PrintSupport ${PLATFORM_SPECIFIC_LIBS}  ${QtMacExtras} ${APPKIT_LIBRARY})
