# Required packages for building on Debian Jessie:
# libboost-all-dev
# libssl-dev
# libgtk2.0-dev
# libfreetype6-dev
# libreadline-dev
# libdb++-dev

#Set just these 2 configs to always produce release build with debug info enabled
# Warning: it must appear before starting project definition
SET (CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebinfo" CACHE STRING "" FORCE)

project( keyhotee )
cmake_minimum_required( VERSION 2.8.12.1 )

IF(WIN32)
   # Build all windows projects with /EHsc, which will prevent the compiler from 
   # translating structured exceptions into C++ exceptions (which would make
   # our crash reporter less useful)
   IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
   ENDIF()

  #Set to allow xp compatibility
  set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "Platform toolset" FORCE)

  #looks like this flag can have different default on some machines.
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")

ENDIF(WIN32)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "7")

IF(NOT DEFINED BUILD_VERSION_PATCH)
  set(VERSION_PATCH "0")
else()
  set(VERSION_PATCH "${BUILD_VERSION_PATCH}")
ENDIF()

SET (INVICTUS_ROOT $ENV{INVICTUS_ROOT})

IF("${INVICTUS_ROOT}" STREQUAL "")
  SET(INVICTUS_ROOT "./")
ENDIF("${INVICTUS_ROOT}" STREQUAL "")

GET_FILENAME_COMPONENT(INVICTUS_ROOT ${INVICTUS_ROOT} REALPATH) # remove ugliness

message(STATUS "Using invictus root: " ${INVICTUS_ROOT})
message(STATUS "Using platform toolset: ${CMAKE_GENERATOR_TOOLSET}")

# Let's configure binaries output directory (by default invictus-root/bin)
if (APPLE)
  # right now we have to put it in a weird location for osx because that's 
  # where jenkins wants it to be for packaging.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/build/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/build/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/build/bin)
else(APPLE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INVICTUS_ROOT}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${INVICTUS_ROOT}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${INVICTUS_ROOT}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${INVICTUS_ROOT}/bin)
endif(APPLE)

set(CMAKE_INSTALL_PREFIX "${INVICTUS_ROOT}/packages" CACHE PATH "Keyhotee install prefix" FORCE)

add_subdirectory(../bitshares_toolkit bitshares_toolkit.bin)
add_subdirectory(BitShares)
add_subdirectory(miner)

macro(FillQtDependentLibraries)
  # Warning this lib doesn't have an associated .cmake :-(
  find_library(Qt5Platform_LIBRARIES_RELEASE Qt5PlatformSupport HINTS "${QT_LIB_DIR}" "${QT_LIB_DIR}/../lib")
  find_library(Qt5Platform_LIBRARIES_DEBUG Qt5PlatformSupport${DEBUG_SUFFIX} HINTS "${QT_LIB_DIR}" "${QT_LIB_DIR}/../lib")

  # Warning this lib doesn't have an associated .cmake :-(
  find_library(Qt5SvgIcon_LIBRARIES_RELEASE qsvgicon HINTS "${QT_LIB_DIR}/../plugins/iconengines/")
  find_library(Qt5SvgIcon_LIBRARIES_DEBUG qsvgicon${DEBUG_SUFFIX} HINTS "${QT_LIB_DIR}/../plugins/iconengines/")
  
  IF(NOT Qt5SvgIcon_LIBRARIES_RELEASE)
    MESSAGE(WARNING "Qt5SvgIcon_LIBRARIES_RELEASE not found, QT plugin static library may not installed, could set KH_STATIC_QT to 0 to disable __STATIC_QT")
  ENDIF(NOT Qt5SvgIcon_LIBRARIES_RELEASE)
  
  if(WIN32)
    message(STATUS "Setting up additional QT Dependencies for Windows platform")
    # put here all system libraries which have to be linked 
    LIST(APPEND "QT_DEPENDENCY_LIBRARIES" "imm32.lib" "winmm.lib")
  else(WIN32)
    IF(UNIX AND NOT APPLE)
      message(STATUS "Setting up additional QT Dependencies for Unix platform")

      find_library(QXcbStatic_LIBRARY xcb-static HINTS "${QT_LIB_DIR}/../plugins/platforms/xcb-static/")
      list (APPEND QT_DEPENDENCY_LIBRARIES ${QXcbStatic_LIBRARY})

      find_library(FONTCONFIG_LIBRARY fontconfig)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${FONTCONFIG_LIBRARY}")

      find_library(FREETYPE_LIBRARY freetype)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${FREETYPE_LIBRARY}")

      find_package (XCB)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${XCB_XCB_LIBRARY}" "${XCB_GLX_LIBRARY}")

      find_package(X11_XCB)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${X11_XCB_LIBRARIES}")

      find_package(X11)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${X11_ICE_LIB}" "${X11_SM_LIB}" "${X11_X11_LIB}"
        "${X11_Xext_LIB}" "${X11_Xi_LIB}" "${X11_Xrender_LIB}"
      )

      find_package(GTK2)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${GTK2_GLIB_LIBRARY_RELEASE}"
       "${GTK2_GOBJECT_LIBRARY_RELEASE}" "${GTK2_GTHREAD_LIBRARY_RELEASE}"
      )
    ENDIF(UNIX AND NOT APPLE)
  ENDIF(WIN32)
endmacro()

set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )
SET( BUILD_SHARED_LIBS OFF )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CmakeModules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/BitShares/CMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bitshares_toolkit/libraries/fc/GitVersionGen")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
get_git_unix_timestamp(GIT_UNIX_TIMESTAMP1)

set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake"  )
#message(CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH})

IF (NOT DEFINED KH_STATIC_QT)
  SET(KH_STATIC_QT 0)
ENDIF(NOT DEFINED KH_STATIC_QT)

SET(QT_DEPENDENCY_LIBRARIES "")
SET(ICU_LIBS_PREFIX "")

SET (ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

SET (DEBUG_SUFFIX "")

MESSAGE (STATUS "Config types: ${CMAKE_CONFIGURATION_TYPES}")

#This variable will be filled just for Win32 platform
SET (CrashRpt_LIBRARIES "")

IF( WIN32 )
  MESSAGE(STATUS "Configuring Keyhotee on WIN32")
  # On windows platform release & debug libraries have different names
  SET (DEBUG_SUFFIX "D")

  # Warning this lib doesn't have an associated .cmake :-(
  find_library(CrashRpt_LIBRARIES_RELEASE CrashRpt1402 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/CrashRpt/lib")

  LIST(APPEND CrashRpt_LIBRARIES optimized ${CrashRpt_LIBRARIES_RELEASE})

  MESSAGE(STATUS "Found CrashRpt libraries: ${CrashRpt_LIBRARIES}")
  set(CRASHRPT_BINARIES_TO_INSTALL "${CMAKE_CURRENT_SOURCE_DIR}/CrashRpt/bin/crashrpt_lang.ini"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/CrashRpt/bin/CrashRpt1402.dll"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/CrashRpt/bin/CrashSender1402.exe"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/CrashRpt/bin/dbghelp.dll")


  IF(KH_STATIC_QT EQUAL 1)
    # Use dedicated prefix to find static versions of libraries 
    SET(ICU_LIBS_PREFIX "s")
  ENDIF()
ELSE()
  IF(KH_STATIC_QT EQUAL 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
  ENDIF()
ENDIF(WIN32)

find_package( ICU )

SET (CMAKE_FIND_LIBRARY_SUFFIXES ${ORIGINAL_LIB_SUFFIXES})

find_package( Qt5LinguistTools )
find_package( Qt5PrintSupport )
find_package( Qt5Svg )
find_package( Qt5Widgets )
find_package( Qt5Network )
find_package( Qt5XmlPatterns )

IF(KH_STATIC_QT EQUAL 0)
  find_package( Qt5WebKit )
ENDIF()

GET_TARGET_PROPERTY(QT_LIB_DIR Qt5::Core LOCATION)
GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)

message("QT-DIRECTORY is ${QT_LIB_DIR}")

#Note: bitshares repo should be checked out under keyhotee dir or path here should be changed
set(BITSHARES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BitShares)
message(STATUS ${BITSHARES_DIR})

IF( APPLE )
    SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")

    SET_SOURCE_FILES_PROPERTIES(
                                "images/keyhotee.icns"
                                PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources
                                )
    SET( MACOSX_BUNDLE_BUNDLE_NAME "Keyhotee" )
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "com.invictus-innovations.Keyhotee" ) 
    set( MACOSX_BUNDLE_ICON_FILE keyhotee.icns )
    SET( MACOSX_BUNDLE_INFO_STRING "Keyhotee - version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2014 Invictus Innovations" ) 

ELSEIF( UNIX )

    SET(CMAKE_CXX_FLAGS "-Wall -std=gnu++11")

ENDIF()

qt5_wrap_ui( profile_wizard/UiProfileEditPage  profile_wizard/ProfileEditPage.ui )
qt5_wrap_ui( profile_wizard/UiProfileIntroPage profile_wizard/ProfileIntroPage.ui )
qt5_wrap_ui( profile_wizard/UiNymPage          profile_wizard/ProfileNymPage.ui )
qt5_wrap_ui( UiLoginDialog                     LoginDialog.ui )
qt5_wrap_ui( UiConnectionStatusFrame           connectionstatusframe.ui )
qt5_wrap_ui( UiKeyhoteeMainWindow              KeyhoteeMainWindow.ui )
qt5_wrap_ui( Uidiagnosticdialog                diagnosticdialog.ui )
qt5_wrap_ui( UiNewIdentityDialog               AddressBook/NewIdentityDialog.ui )
qt5_wrap_ui( UiContactsTable                   AddressBook/ContactsTable.ui )
qt5_wrap_ui( UiContactView                     AddressBook/ContactView.ui )
qt5_wrap_ui( UiRequestAuthorization            AddressBook/RequestAuthorization.ui )
qt5_wrap_ui( UiKeyhoteeIDPubKeyWidget          AddressBook/keyhoteeidpubkeywidget.ui )
qt5_wrap_ui( UiAuthorization                   AddressBook/authorization.ui )
qt5_wrap_ui( UiIdentitySelection               Identity/IdentitySelection.ui )
qt5_wrap_ui( UiBlockedContentAlert             Mail/BlockedContentAlert.ui )
qt5_wrap_ui( UiMailbox                         Mail/Mailbox.ui )
qt5_wrap_ui( UiMailViewer                      Mail/MailViewer.ui )
qt5_wrap_ui( UiMailEditor                      Mail/maileditorwindow.ui )
qt5_wrap_ui( UiMailFieldsWidgets               Mail/mailfieldswidget.ui )
qt5_wrap_ui( UiMoneyAttachementWidget          Mail/moneyattachementwidget.ui )
qt5_wrap_ui( UiFileAttachmentWidget            Mail/fileattachmentwidget.ui )
qt5_wrap_ui( UiFileAttachmentDialog            Mail/FileAttachmentDialog.ui )
qt5_wrap_ui( UiWallets                         Wallets/wallets.ui )
qt5_wrap_ui( UiOptionsDialog                   Options/OptionsDialog.ui )
qt5_wrap_ui( UiLoginInputBox                   qtreusable/LoginInputBox.ui )
qt5_wrap_ui( UiHeaderWidget                    qtreusable/HeaderWidget.ui )

set( sources  
        qtreusable/selfsizingmainwindow.h
        qtreusable/selfsizingmainwindow.cpp
        qtreusable/AutoUpdateProgressBar.hpp
        qtreusable/AutoUpdateProgressBar.cpp
        qtreusable/TImage.hpp
        qtreusable/TImage.cpp
        qtreusable/MimeDataChecker.hpp
        qtreusable/MimeDataChecker.cpp
        qtreusable/LoginInputBox.ui
        qtreusable/LoginInputBox.hpp
        qtreusable/LoginInputBox.cpp
        qtreusable/HeaderWidget.ui
        qtreusable/HeaderWidget.hpp
        qtreusable/HeaderWidget.cpp

        profile_wizard/ProfileEditPage.ui
        profile_wizard/ProfileIntroPage.ui
        profile_wizard/ProfileNymPage.ui
        profile_wizard/ProfileWizard.hpp
        profile_wizard/ProfileWizard.cpp

        AddressBook/AddressBookModel.hpp
        AddressBook/AddressBookModel.cpp
        AddressBook/NewIdentityDialog.ui
        AddressBook/NewIdentityDialog.hpp
        AddressBook/NewIdentityDialog.cpp
        AddressBook/ContactsTable.ui
        AddressBook/ContactsTable.hpp
        AddressBook/ContactsTable.cpp
        AddressBook/ContactView.ui
        AddressBook/ContactView.hpp
        AddressBook/ContactView.cpp
        AddressBook/Contact.cpp
        AddressBook/Contact.hpp
        AddressBook/ContactCompleterModel.hpp
        AddressBook/ContactCompleterModel.cpp
        AddressBook/TableViewCustom.hpp
        AddressBook/TableViewCustom.cpp
        AddressBook/RequestAuthorization.ui
        AddressBook/RequestAuthorization.hpp
        AddressBook/RequestAuthorization.cpp
        AddressBook/keyhoteeidpubkeywidget.ui
        AddressBook/keyhoteeidpubkeywidget.hpp
        AddressBook/keyhoteeidpubkeywidget.cpp
        AddressBook/authorization.ui
        AddressBook/authorization.hpp
        AddressBook/authorization.cpp
        AddressBook/ContactvCard.hpp
        AddressBook/ContactvCard.cpp
        AddressBook/ContactGui.hpp
        AddressBook/ContactGui.cpp
        AddressBook/AuthorizationItem.hpp
        AddressBook/AuthorizationItem.cpp

        Identity/IdentitySelection.ui
        Identity/IdentitySelection.hpp
        Identity/IdentitySelection.cpp
        Identity/IdentitiesUpdate.hpp
        Identity/IdentityObservable.hpp
        Identity/IdentityObservable.cpp

        Mail/BlockerDelegate.hpp
        Mail/BlockedContentAlert.ui
        Mail/BlockedContentAlert.hpp
        Mail/BlockedContentAlert.cpp
        Mail/FileAttachmentDialog.ui
        Mail/FileAttachmentDialog.hpp
        Mail/FileAttachmentDialog.cpp
        Mail/fileattachmentwidget.ui
        Mail/fileattachmentwidget.hpp
        Mail/fileattachmentwidget.cpp
        Mail/Mailbox.ui
        Mail/Mailbox.hpp
        Mail/Mailbox.cpp
        Mail/MailboxModel.hpp
        Mail/MailboxModel.cpp
        Mail/MailboxModelRoot.hpp
        Mail/MailboxModelRoot.cpp
        Mail/maileditorwindow.ui
        Mail/maileditorwindow.hpp
        Mail/maileditorwindow.cpp
        Mail/mailfieldswidget.ui
        Mail/mailfieldswidget.hpp
        Mail/mailfieldswidget.cpp
        Mail/MailTable.hpp
        Mail/MailTable.cpp
        Mail/MailViewer.ui
        Mail/MailViewer.hpp
        Mail/MailViewer.cpp
        Mail/MessageHeader.hpp
        Mail/moneyattachementwidget.ui
        Mail/moneyattachementwidget.hpp
        Mail/moneyattachementwidget.cpp
        Mail/RepliedMailPattern.html
        Mail/ShowColumnAction.hpp
        Mail/ShowColumnAction.cpp
        Mail/TableWidgetAttachments.hpp
        Mail/TableWidgetAttachments.cpp
        Mail/TMessageEdit.hpp
        Mail/TMessageEdit.cpp

        KeyhoteeApplication.hpp
        KeyhoteeApplication.cpp

        LoginDialog.ui
        LoginDialog.hpp
        LoginDialog.cpp

        ContactListEdit.hpp
        ContactListEdit.cpp

        GitSHA1.h

        TreeWidgetCustom.hpp
        TreeWidgetCustom.cpp

        KeyhoteeMainWindow.ui
        KeyhoteeMainWindow.hpp
        KeyhoteeMainWindow.cpp

        Wallets/ManagedStream.hpp
        Wallets/ManagedStream.cpp
        Wallets/ManageWallet.hpp
        Wallets/ManageWallet.cpp
        Wallets/wallets.ui
        Wallets/wallets.hpp
        Wallets/wallets.cpp
        Wallets/WalletsGui.hpp
        Wallets/WalletsGui.cpp
        Wallets/XmlWalletsReader.hpp
        Wallets/XmlWalletsReader.cpp
        Wallets/XmlMessageHandler.hpp
        Wallets/XmlMessageHandler.cpp

        ATopLevelWindowsContainer.hpp
        ATopLevelWindowsContainer.cpp
        ATopLevelWindow.hpp
        ATopLevelWindow.cpp

        bitsharesguiprecomp.h

        diagnosticdialog.ui
        diagnosticdialog.h
        diagnosticdialog.cpp

        utils.hpp
        utils.cpp

        connectionstatusframe.ui
        connectionstatusframe.h
        connectionstatusframe.cpp

        public_key_address.hpp

        ConnectionProcessor.hpp
        ConnectionProcessor.cpp

        ch/authprocessor.hpp
        ch/connectionstatusds.h
        ch/mailprocessor.hpp
        ch/ModificationsChecker.hpp

        MenuEditControl.hpp
        MenuEditControl.cpp

        vCard/vcardproperty.h
        vCard/vcardparam.h
        vCard/vcard.h
        vCard/libvcard_global.h
        vCard/vcardproperty.cpp
        vCard/vcardparam.cpp
        vCard/vcard.cpp

        Options/OptionsDialog.ui
        Options/OptionsDialog.h
        Options/OptionsDialog.cpp

        main.cpp )

set(FILES_TO_TRANSLATE ${sources})
file (GLOB TRANSLATIONS_FILES translations/*.ts)
option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts
  files (WARNING: make clean will delete the source .ts files! Danger!)")
if (UPDATE_TRANSLATIONS)
  qt5_create_translation(QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt5_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS) 

add_custom_target (translations_target DEPENDS ${QM_FILES})

# Construct an appropriate resource file
SET(QM_QRC "<RCC>\n<qresource prefix=\"/\">\n")
FOREACH(QM_FILE ${QM_FILES})
FILE(RELATIVE_PATH QM_FILE ${CMAKE_CURRENT_BINARY_DIR} ${QM_FILE})
SET(QM_QRC "${QM_QRC}<file>${QM_FILE}</file>\n")
ENDFOREACH(QM_FILE ${QM_FILES})
SET(QM_QRC "${QM_QRC}</qresource>\n</RCC>\n")
SET(QM_QRC_PATH ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_trans.qrc)
FILE(WRITE ${QM_QRC_PATH} ${QM_QRC})

qt5_add_resources( KeyhoteeQRC  Keyhotee.qrc  ${QM_QRC_PATH})

set( resources
        Keyhotee.qrc
        keyhotee_trans.qrc
        images/keyhotee.icns
        qrc_keyhotee_trans.cpp 
        qrc_Keyhotee.cpp )


# Append files generated from git revision to the sources list
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" GitSHA1.h)
list(APPEND sources ${headers})

if(WIN32)
  # add an icon to the win32 executable
  list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/Keyhotee.rc)
endif(WIN32)

# Some unknown diff between windows and unix, this is quick fix, leave for DanL to clean-up
IF(NOT WIN32)
  #apparently this is needed for Ubuntu (see commit from jefferylee), but it causes problems for windows (lib is brought in via upnpc-static on windows at least)
  set(MINIUPNP_LIB  miniupnpc)
ENDIF(NOT WIN32)

# Configure dependencies of keyhotee main application module
add_executable( Keyhotee WIN32 MACOSX_BUNDLE ${sources} ${resources} )
add_dependencies (Keyhotee translations_target)
add_dependencies(Keyhotee bitshares_client)

get_target_property(FC_SHARED_LIBRARIES_DEBUG fc SHARED_LIBRARIES_DEBUG)
get_target_property(FC_SHARED_LIBRARIES_RELEASE fc SHARED_LIBRARIES_RELEASE)

# Looks like multiple post build commands doesn't work correctly (last one overwrites previous defs)
SET(POST_BUILD_STEP_COMMANDS )

IF(WIN32)
  foreach(binary ${CRASHRPT_BINARIES_TO_INSTALL})
    set(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${binary}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  endforeach()
  
  IF(FC_SHARED_LIBRARIES_DEBUG)
    foreach(binary ${FC_SHARED_LIBRARIES_DEBUG})
      set(POST_BUILD_STEP_COMMANDS
       ${POST_BUILD_STEP_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_if_different "${binary}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endforeach()
  ENDIF()
  
  IF(FC_SHARED_LIBRARIES_RELEASE)
    foreach(binary ${FC_SHARED_LIBRARIES_RELEASE})
      set(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${binary}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endforeach()
  ENDIF()
ENDIF()

target_include_directories(Keyhotee
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
  )

#Pass define to disable some GUI functionality (being in dev) only in release
set_target_properties( Keyhotee PROPERTIES COMPILE_DEFINITIONS_RELEASE ALPHA_RELEASE )
set_target_properties( Keyhotee PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO ALPHA_RELEASE )

IF(KH_STATIC_QT EQUAL 1)
  target_compile_definitions(Keyhotee PRIVATE __STATIC_QT)
  FillQtDependentLibraries()
ENDIF()

# Let's configure different target names for release and debug builds.
set_target_properties( Keyhotee PROPERTIES OUTPUT_NAME Keyhotee DEBUG_POSTFIX D )
set_target_properties( Keyhotee PROPERTIES INSTALL_RPATH ".;")

set (PDB_LIST_RELEASE)
set (PDB_LIST_DEBUG)

get_target_property(PDB_LIST_RELEASE fc INTERFACE_LINK_PDB_RELEASE)
get_target_property(PDB_LIST_DEBUG fc INTERFACE_LINK_PDB_DEBUG)

# Use the Widgets module from Qt 5.
IF(KH_STATIC_QT EQUAL 0)
  qt5_use_modules(Keyhotee Svg Widgets Gui Core PrintSupport XmlPatterns WebKitWidgets)
ELSE()
  qt5_use_modules(Keyhotee Svg Widgets Gui Core PrintSupport XmlPatterns Network)
ENDIF()

set(CMAKE_STATIC_TYPE true)

#fc  miniupnpc
IF (KH_STATIC_QT EQUAL 1)
  target_link_libraries( Keyhotee
    ${Qt5Gui_PLUGINS} 
    ${CrashRpt_LIBRARIES}
    debug ${Qt5Platform_LIBRARIES_DEBUG} optimized ${Qt5Platform_LIBRARIES_RELEASE}
    debug ${Qt5SvgIcon_LIBRARIES_DEBUG} optimized ${Qt5SvgIcon_LIBRARIES_RELEASE} 
    Qt5::Svg Qt5::Widgets Qt5::Gui Qt5::PrintSupport Qt5::Network
    # Warning this is old Bitshares module
    bshare 
    # Warning this is a bitshare_toolkit module
    #bts_blockchain
    #bts_wallet
# Since we would like to build KH against static versions of ICU, it is needed to specify them in
# valid order (DT must be at the end)
    ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${QT_DEPENDENCY_LIBRARIES} ${ICU_LIBRARIES} ${ICU_DT_LIBRARY}
    ${APPKIT_LIBRARY} ${CMAKE_DL_LIBS} 
  )
# without call FillQtDependentLibraries micro
ELSE (KH_STATIC_QT EQUAL 1)
  target_link_libraries( Keyhotee
    Qt5::Svg Qt5::Widgets Qt5::Gui Qt5::PrintSupport
    ${CrashRpt_LIBRARIES}
    # Warning this is old Bitshares module
    bshare 
    # Warning this is a bitshare_toolkit module
    #bts_blockchain
    #bts_wallet
    ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${QT_DEPENDENCY_LIBRARIES} ${ICU_LIBRARIES}
    ${APPKIT_LIBRARY} ${CMAKE_DL_LIBS} 
  )
ENDIF (KH_STATIC_QT EQUAL 1)

INSTALL(TARGETS Keyhotee RUNTIME DESTINATION . BUNDLE DESTINATION . COMPONENT Runtime CONFIGURATIONS RelWithDebinfo)

if (MSVC)
  message(STATUS "Setting up debug options for MSVC build")
# Probably cmake has a bug and vcxproj generated for executable in Debug conf. has disabled debug info
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
endif(MSVC)

GET_TARGET_PROPERTY(_loc Keyhotee LOCATION_RELWITHDEBINFO)
get_filename_component(_binName ${_loc} NAME)
get_filename_component(_binPath ${_loc} PATH)

IF(WIN32)
  #Warning Looks like windows NSIS has some problem related to proper detecting target binary while installing it
  #and it is missing in the target package location, what leads bundle verification to fail.
  INSTALL(FILES "${_loc}" DESTINATION . COMPONENT Runtime CONFIGURATIONS RelWithDebinfo)
  INSTALL(FILES "${_binPath}/openssl.cnf" DESTINATION . COMPONENT Runtime CONFIGURATIONS RelWithDebinfo)
ENDIF(WIN32)

IF(APPLE)
  SET(plugin_dest_dir Keyhotee.app/Contents/MacOS)
  SET(qtconf_dest_dir Keyhotee.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/Keyhotee.app")
else()
  SET(plugin_dest_dir .)
  SET(qtconf_dest_dir .)

  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${_binName}")
ENDIF(APPLE)

message(STATUS "KH output path: ${APPS}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_SHARED_LIBRARY_SUFFIX: ${CMAKE_SHARED_LIBRARY_SUFFIX}")

IF(FC_SHARED_LIBRARIES_RELEASE)
  INSTALL(FILES ${FC_SHARED_LIBRARIES_RELEASE} DESTINATION . CONFIGURATIONS RelWithDebinfo)
ENDIF()

foreach(plugin ${Qt5Gui_PLUGINS})
  get_target_property(_loc ${plugin} LOCATION)
  get_filename_component(plugin_name ${_loc} NAME_WE)
  get_filename_component(plugin_ext ${_loc} EXT)
  get_filename_component(plugin_path ${_loc} DIRECTORY)
  get_filename_component(plugin_classification ${plugin_path} NAME)

#  message(STATUS "Preparing a copy for plugin ${plugin} (${plugin_name}) at location ${_loc}")
  SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${plugin_path}/${plugin_name}$<$<CONFIG:Debug>:$<TARGET_PROPERTY:Keyhotee,DEBUG_POSTFIX>>${plugin_ext}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/${plugin_classification}/${plugin_name}$<$<CONFIG:Debug>:$<TARGET_PROPERTY:Keyhotee,DEBUG_POSTFIX>>${plugin_ext}")
endforeach()

get_target_property(_loc bitshares_client LOCATION_RELWITHDEBINFO)
SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_loc}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../bitshares_toolkit/programs/qt_wallet/htdocs" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/htdocs")

set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .) # copy the msvcrt files to the top-level directory instead of bin
include(InstallRequiredSystemLibraries)

SET (QTPLUGINS )
foreach(plugin ${Qt5Gui_PLUGINS})
  get_target_property(_loc ${plugin} LOCATION)
  get_filename_component(plugin_path ${_loc} DIRECTORY)
  get_filename_component(plugin_classification ${plugin_path} NAME)
  #message("Plugin ${plugin} is an ${plugin_classification} at location ${_loc}")
  list(APPEND QTPLUGINS "${_loc}") 
  INSTALL(FILES ${_loc} DESTINATION ${plugin_dest_dir}/plugins/${plugin_classification} COMPONENT Runtime) 
endforeach()

get_target_property(_loc bitshares_client LOCATION_RELWITHDEBINFO)
INSTALL(FILES ${_loc} DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

INSTALL(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/htdocs" DESTINATION "${plugin_dest_dir}/plugins" COMPONENT Runtime)

INSTALL(CODE "
  file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
  INCLUDE(BundleUtilities)
  FIXUP_BUNDLE(\"${APPS}\" \"\${QTPLUGINS}\" \"\${QT_LIB_DIR}\")
  " COMPONENT Runtime)

# Create an empty qt.conf (similary to installation process) to avoid startup problems and looking for plugins dir.
SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
  COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf")

IF(WIN32)
  INSTALL(FILES ${CRASHRPT_BINARIES_TO_INSTALL} DESTINATION . CONFIGURATIONS RelWithDebinfo COMPONENT Runtime)

  get_target_property(FC_INTERFACE_PDB_LIST_RELEASE fc INTERFACE_LINK_PDB_RELEASE)

  SET(TOTAL_PDB_LIST )
  IF(FC_INTERFACE_PDB_LIST_RELEASE)
    LIST (APPEND TOTAL_PDB_LIST ${FC_INTERFACE_PDB_LIST_RELEASE})
  ENDIF()

  set(KH_PDB_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:Keyhotee,OUTPUT_NAME>$<$<CONFIG:Debug>:$<TARGET_PROPERTY:Keyhotee,DEBUG_POSTFIX>>.pdb")
  LIST (APPEND TOTAL_PDB_LIST ${KH_PDB_PATH})

  message(STATUS "TOTAL_PDB_LIST: ${TOTAL_PDB_LIST}")

  IF(TOTAL_PDB_LIST)
    set(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/")
    FOREACH(pdb ${TOTAL_PDB_LIST})
      set(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${pdb}" "${CMAKE_INSTALL_PREFIX}/")
    endforeach()
  ENDIF(TOTAL_PDB_LIST)
ELSE(WIN32)
  IF(NOT APPLE)
    FIND_LIBRARY(_stdCPPLibrary libstdc++.so.6 libstdc++.so stdc++)
    MESSAGE(STATUS "FOUND STDC++ LIBRARY: ${_stdCPPLibrary}")
    INSTALL(FILES ${_stdCPPLibrary} DESTINATION . CONFIGURATIONS RelWithDebinfo COMPONENT Runtime)
    FIND_LIBRARY(_stdCPPLibrary18 libstdc++.so.6.0.18)
    MESSAGE(STATUS "FOUND STDC++ LIBRARY: ${_stdCPPLibrary18}")
    INSTALL(FILES ${_stdCPPLibrary18} DESTINATION . CONFIGURATIONS RelWithDebinfo COMPONENT Runtime)

  ENDIF(NOT APPLE)
ENDIF(WIN32)

ADD_CUSTOM_COMMAND(TARGET Keyhotee POST_BUILD ${POST_BUILD_STEP_COMMANDS} 
  COMMENT "Copying Qt plugins, binaries and other files into target directory."
)

SET(CPACK_PACKAGE_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
set(CPACK_PACKAGE_NAME "keyhotee")
set(CPACK_PACKAGE_VENDOR "Invictus Innovations, Inc.")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION "A secure digital messaging platform and electronic wallet")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A secure digital messaging platform")
set(CPACK_PACKAGE_EXECUTABLES "Keyhotee;Keyhotee")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/BitShares/LICENSE.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Keyhotee ${CPACK_PACKAGE_VERSION}")

if(WIN32)
 SET(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_PACKAGE_NAME "Keyhotee") # override above
 set(CPACK_NSIS_CONTACT "contact.us@invictus-innovations.com")
 set(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
 set(CPACK_NSIS_PACKAGE_NAME "Keyhotee v${CPACK_PACKAGE_VERSION}")
 set(CPACK_NSIS_DISPLAY_NAME "${CPACK_NSIS_PACKAGE_NAME}")
 set(CPACK_NSIS_URL_INFO_ABOUT "http://invictus.io")
 set(CPACK_NSIS_HELP_LINK "https://bitsharestalk.org/index.php?board=2.0")
 set(CPACK_NSIS_MENU_LINKS
     "https://bitsharestalk.org/index.php?board=2.0" "Keyhotee Development Forum"
     "http://invictus.io" "Invictus Innovations Website")
 set(CPACK_NSIS_DEFINES "  !define MUI_STARTMENUPAGE_DEFAULTFOLDER \\\"Keyhotee\\\"")
 # it seems like windows zip files usually don't have a single directory inside them, unix tgz frequently do
 SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

endif(WIN32)

if(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_PACKAGE_VENDOR "www.invictus.io")
endif()

if(UNIX)
  # Linux gets a .tgz
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
  SET(CPACK_STRIP_FILES 1)
endif(UNIX)

include(CPack)


