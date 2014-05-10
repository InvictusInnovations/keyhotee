rem this script should be spawned from directory containing OpenSSL sources.
rem built library will be installed to %INVICTUS_ROOT%/OpenSSL

set INVICTUS_ROOT=%~dp0..\..\
echo Using %INVICTUS_ROOT% as Invictus root directory

call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"

set PREFIX=..\OpenSSL

patch -d util/pl -i %INVICTUS_ROOT%\keyhotee\HelperScripts\VC-32.pl.patch

perl Configure VC-WIN32 --prefix=%PREFIX%

call ms\do_nasm.bat
nmake -f ms\nt.mak
nmake -f ms\nt.mak install

nmake clean
perl Configure debug-VC-WIN32 --prefix=%PREFIX%
call ms\do_nasm.bat
nmake -f ms\nt.mak
nmake -f ms\nt.mak install

REM md %PREFIX%\lib\VC\static
REM copy /Y %PREFIX%\lib\*.* %PREFIX%\lib\VC\static
REM del /Q /F %PREFIX%\lib\*.*
