project( keyhotee )
cmake_minimum_required( VERSION 2.8.11 )

set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GitVersionGen/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

IF( WIN32 )
#  set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake;C:/gh/qt5.1.1/qtwebkit/WebKitBuild/Release/lib/cmake/Qt5WebKitWidgets;"  )
  set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake"  )
  message(CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH})

  #You need to set OPENSSL_ROOT environment variable for your system on WIN32
  message("Setting up OpenSSL root and include vars on Win32 platform")
  set( OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT} )
  set( OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include )

ENDIF(WIN32)

find_package( OpenSSL )
find_package( Qt5Core )
find_package( Qt5WebKitWidgets )
find_package( Qt5PrintSupport )

if (WIN32)
  MESSAGE(platform= "Compiling on WIN32")
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
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -DLEVELDB_PLATFORM_WINDOWS )
  # Activate C++ exception handling
  IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  ENDIF()
  #boost
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  add_definitions(-DBOOST_ALL_NO_LIB)  # Needed to disable MSVC autolinking feature (#pragma comment)
  set(Boost_USE_DEBUG_PYTHON ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_LIBS OFF)
  set(BOOST_ALL_DYN_LINK ON) # force dynamic linking for all libraries

  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context) 

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

include_directories( ${OPENSSL_INCLUDE_DIR} )
include_directories( ${Boost_INCLUDE_DIR} )
SET( ALL_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES})

add_subdirectory(bitshares)
add_subdirectory(miner)

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
qt5_wrap_ui( UiConnectionStatusFrame           connectionstatusframe.ui )
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

        qtreusable/selfsizingmainwindow.cpp

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
        AddressBook/Contact.cpp

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

        connectionstatusframe.ui
        connectionstatusframe.h
        connectionstatusframe.cpp

        dataaccessimpl.h
        dataaccessimpl.cpp

        ch/connectionstatusds.h

        main.cpp )

# Append files generated from git revision to the sources list
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" GitSHA1.h)

# Configure dependencies of keyhotee_library module
add_library( keyhotee_library ${library_sources} )
qt5_use_modules(keyhotee_library Widgets WebKit)
message(BOOST_LIBRARIES=${BOOST_LIBRARIES})
target_link_libraries( keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${APPKIT_LIBRARY})

# Let's configure binaries output directory (by default invictus-root/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{INVICTUS_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE $ENV{INVICTUS_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG $ENV{INVICTUS_ROOT}/bin)

# Configure dependencies of keyhotee main application module
add_executable( Keyhotee WIN32 MACOSX_BUNDLE ${sources} )

# Let's configure different target names for release and debug builds.
set_target_properties( Keyhotee PROPERTIES OUTPUT_NAME_DEBUG KeyhoteeD )
set_target_properties( Keyhotee PROPERTIES OUTPUT_NAME_RELEASE Keyhotee )

# Use the Widgets module from Qt 5.
qt5_use_modules(Keyhotee Widgets PrintSupport WebKit)

target_link_libraries( Keyhotee keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${APPKIT_LIBRARY})

if (MSVC)
  message("Setting up debug options for MSVC build")
# Probably cmake has a bug and vcxproj generated for executable in Debug conf. has disabled debug info
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
endif(MSVC)

