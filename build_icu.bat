rem All operations made here are related to instructions: http://qt-project.org/wiki/Compiling-ICU-with-MSVC
@echo off
REM - there is created root directory (ICU_ROOT)
REM - inside them is extracted icu4c-53_1-src.zip
REM - ICU_ROOT/sources contains all ICU sources.
REM - ICU_ROOT/build is a cwd and this script must be started from it
REM To preserve debug info (in produced static libraries) lets open source\runConfigureICU, find
REM Cygwin/MSVC) platform section and change /Zi switches to /Z7

setlocal

rem This variable must be set manually.
set CYGWIN_ROOT=c:\Tools\cygwin
set CL=/MP

set ICU_ROOT=%cd%\..
echo Using ICU_ROOT: %ICU_ROOT%

set PATH=%ICU_ROOT%\bin;%CYGWIN_ROOT%\bin;%PATH%

for /f "delims=" %%a in ('cygpath %ICU_ROOT%') do @set ICU_PREFIX=%%a

echo ICU prefix: %ICU_PREFIX%

call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"

dos2unix *
dos2unix -f configure

rem Configuring release & debug versions

bash ../source/runConfigureICU --enable-debug --disable-release Cygwin/MSVC --prefix=%ICU_PREFIX% ^
 2>&1 | tee ../configure_debug.log && ^
make -j%NUMBER_OF_PROCESSORS% 2>&1 | tee ../build_debug.log && ^
make install 2>&1 | tee ../install_debug.log

cd %ICU_ROOT%
bash -c "rm -rf %ICU_PREFIX%/build"
mkdir build
cd %ICU_ROOT%\build

bash ../source/runConfigureICU Cygwin/MSVC --prefix=%ICU_PREFIX% 2>&1 | tee ../configure_release.log && ^
make -j%NUMBER_OF_PROCESSORS% 2>&1 | tee ../build_release.log && ^
make install 2>&1 | tee ../install_release.log


