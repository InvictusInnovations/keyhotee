rem this script runs cmake to generate project files accoring to CMakeLists specifications.
rem Should be started from directory where build should be performed (ie %INVICTUS_ROOT%\vs12)
setlocal
call %~dp0\setenv.bat
rem cd %INVICTUS_ROOT%
rem -DFORCE_BUILDWEB_GENERATION=TRUE 
cmake -DINCLUDE_QT_WALLET=TRUE -G "Visual Studio 12" "%INVICTUS_ROOT%\keyhotee" -T "v120_xp" "%INVICTUS_ROOT%\keyhotee" %*
endlocal

