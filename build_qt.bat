REM Assumptions: 
REM - there is created root directory (QTDIR)
REM - inside them is created directory 'build' to perform shadow build. This script should be started from this directory
REM - QTDIR/src contains all QT sources
REM Before building QT statically you can edit src\qtbase\mkspecs\win32-msvc2013\qmake.conf and
REM change all -Zi options to -Z7 to avoid debug info loss after deleting build directory.
@echo off
setlocal

set CL=/MP
set QTDIR=%CD%\..

set GNUWIN32_ROOT=%QTDIR%\src\gnuwin32
set OPENSSL_ROOT=f:\SCAD\lib\openssl
set ICU_ROOT=%QTDIR%\..\ICU

set PATH=%ICU_ROOT%\bin;%ICU_ROOT%\lib;%GNUWIN32_ROOT%\bin;%PATH%
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"

call ../src/configure.bat -nomake tests -sse2 -no-sse3 -no-avx -no-avx2 -no-ssse3 ^
-no-sse4.1 -no-sse4.2 -prefix %QTDIR% -confirm-license -opensource -no-compile-examples -nomake examples ^
-platform win32-msvc2013 -mp -debug-and-release -force-debug-info -icu -I %ICU_ROOT%\include -L %ICU_ROOT%\lib ^
-openssl -I %OPENSSL_ROOT%\inc32 -L %OPENSSL_ROOT%\out32 -I %QTDIR%/src/qtbase/src/3rdparty/zlib -accessibility ^
-qt-sql-sqlite -qt-zlib -qt-libpng -qt-libjpeg -no-vcproj -plugin-manifests -qmake -process -rtti ^
-audio-backend -qt-style-windows -qt-style-windowsxp -qt-style-windowsvista -no-style-windowsce ^
-no-style-windowsmobile -native-gestures 2>&1 | tee configure.log && ^
nmake 2>&1 | tee build.log && ^
nmake install 2>&1 | tee install.log

endlocal

