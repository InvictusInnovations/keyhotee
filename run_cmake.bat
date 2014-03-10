setlocal
call setenv.bat
cd %INVICTUS_ROOT%
cmake-gui -G "Visual Studio 11"
