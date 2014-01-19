REM Assumptions: 
REM - there is created root directory (QTDIR)
REM - inside them is created directory 'build' to perform shadow build. This script should be started from this directory
REM - QTDIR/src contains all QT sources

@echo off
setlocal

set CL=/MP
set QTDIR=%CD%\..

set GNUWIN32_ROOT=%QTDIR%\src\gnuwin32
set OPENSSL_ROOT=f:\SCAD\lib\openssl
set ICU_ROOT=%QTDIR%\..\ICU

set PATH=%ICU_ROOT%\bin;%GNUWIN32_ROOT%\bin;%PATH%
call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat"

call ../src/configure.bat -static -no-c++11 -skip qtwebkit -sse2 -no-sse3 -no-avx -no-avx2 -no-ssse3 ^
-no-sse4.1 -no-sse4.2 -prefix %QTDIR% -no-opengl -no-compile-examples -nomake examples -nomake tests -confirm-license -opensource ^
-platform win32-msvc2012 -mp -debug-and-release -force-debug-info -icu -I %ICU_ROOT%\include -L %ICU_ROOT%\lib ^
-openssl -I %OPENSSL_ROOT%\inc32 -L %OPENSSL_ROOT%\out32 -I %QTDIR%/src/qtbase/src/3rdparty/zlib -accessibility ^
-qt-sql-sqlite -no-openvg -qt-zlib -qt-libpng -qt-libjpeg -no-vcproj -plugin-manifests -qmake -process -rtti ^
-audio-backend -qt-style-windows -qt-style-windowsxp -qt-style-windowsvista -no-style-windowsce ^
-no-style-windowsmobile -native-gestures 2>&1 | tee configure.log && ^
nmake 2>&1 | tee build.log && ^
nmake install 2>&1 | tee install.log

endlocal

