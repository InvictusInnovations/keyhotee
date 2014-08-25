@echo off
set INVICTUS_ROOT=%~dp0..\
echo Using %INVICTUS_ROOT% as Invictus root directory

set QTDIR=%INVICTUS_ROOT%\QT
set OPENSSL_ROOT=%INVICTUS_ROOT%\OpenSSL
set OPENSSL_ROOT_DIR=%OPENSSL_ROOT%
set OPENSSL_INCLUDE_DIR=%OPENSSL_ROOT%\include
set ICUROOT=%INVICTUS_ROOT%\ICU
if "%DBROOTDIR%" == "" set DBROOTDIR=%INVICTUS_ROOT%\BerkeleyDB
set TCL_ROOT=%INVICTUS_ROOT%\tcl

rem set BOOST_ROOT only if it is not yet configured
rem if "%BOOST_ROOT%" == "" set BOOST_ROOT=%INVICTUS_ROOT%\boost
set BOOST_ROOT=%INVICTUS_ROOT%\boost_1.55

set PATH="%APPDATA%\npm";%QTDIR%\bin;%ICUROOT%\bin;%ICUROOT%\lib;%INVICTUS_ROOT%\bin;%INVICTUS_ROOT%\tcl;%INVICTUS_ROOT%\Cmake\bin;%PATH%

echo Setting up VS2013 environment...
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"

