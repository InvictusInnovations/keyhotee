setlocal
call %~dp0\setenv.bat
cd %INVICTUS_ROOT%
cmake-gui -G "Visual Studio 12"
