setlocal

call "%~dp0\..\setenv.bat"

rem goto doBld
pushd "%INVICTUS_ROOT%"
echo Updating bitshares_toolkit sources...

If exist bitshares_toolkit ( 
    pushd bitshares_toolkit
    git pull || exit /b 1
) else (
    git clone https://github.com/BitShares/bitshares_toolkit.git || exit /b 2
    pushd bitshares_toolkit
	git submodule init
	git submodule update
)

echo Updating leveldb-win sources...

pushd vendor
if exist leveldb-win/.git ( 
    pushd leveldb-win
    git pull || exit /b 3
    popd
) else (
    git clone https://www.github.com/InvictusInnovations/leveldb-win.git || exit /b 4
)

rem popd to be in bitshares_toolkit
popd
rem popd to be in INVICTUS_ROOT dir
popd

pushd "keyhotee"
echo Updating Bitshares sources...

If exist BitShares ( 
    pushd BitShares
    git pull || exit /b 5
	popd
) else (
    git clone https://www.github.com/InvictusInnovations/BitShares.git || exit /b 6
)

rem popd to be in startup dir
popd

:doBld
echo Checking build directory
IF EXIST "%INVICTUS_ROOT%\build" (
  echo "Removing build directory"
  rmdir /Q /S "%INVICTUS_ROOT%\build" || exit /b 7
)

echo Checking packages directory: %INVICTUS_ROOT%\packages

IF EXIST "%INVICTUS_ROOT%\packages" (
  echo Removing packages directory
  del /F /Q "%INVICTUS_ROOT%\packages"
  rmdir /Q /S "%INVICTUS_ROOT%\packages" || exit /b 8
)

mkdir "%INVICTUS_ROOT%\packages" || exit /b 9

if "%1" == "" (
  set BUILD_NUMBER=0
) else (
  set BUILD_NUMBER=%1
)

echo Creating build directory
mkdir "%INVICTUS_ROOT%\build" || exit /b 10
pushd "%INVICTUS_ROOT%\build"
echo "Spawning cmake generator"
call "%INVICTUS_ROOT%\keyhotee\run_cmake.bat" -DBUILD_VERSION_PATCH=%BUILD_NUMBER% || exit /b 11
rem /p:VCTargetsPath="C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V110/"
msbuild.exe /M:%NUMBER_OF_PROCESSORS% /p:Configuration=RelWithDebinfo /p:Platform=Win32 /target:rebuild /clp:ErrorsOnly keyhotee.sln
set MSBUILD_STATUS=%ERRORLEVEL% 
if not %MSBUILD_STATUS%==0 exit /b 12
cpack --verbose || exit /b 13

pushd "%INVICTUS_ROOT%/packages"
tar -czf Keyhotee_build_%BUILD_NUMBER%.tgz *.zip *.pdb || exit /b 14
rem to be in startup dir
popd 

endlocal
