call %~dp0\..\setenv.bat
goto doBld
pushd "%INVICTUS_ROOT%\keyhotee"

If exist BitShares ( 
    pushd BitShares
    git pull || exit /b 1
) else (
    git clone https://www.github.com/InvictusInnovations/BitShares.git || exit /b 2
    pushd BitShares
)

if exist fc ( 
    pushd fc
    git pull || exit /b 3
    popd
) else (
    git clone https://www.github.com/InvictusInnovations/fc.git || exit /b 4
)

pushd vendor
if exist leveldb-win/.git ( 
    pushd leveldb-win
    git pull || exit /b 5
    popd
) else (
    git clone https://www.github.com/InvictusInnovations/leveldb-win.git || exit /b 6
)

rem popd to be in Bitshares
popd 
rem popd to be in keyhotee
popd 
rem popd to be in startup dir
popd

:doBld
echo "Checking build directory"
if exist "%INVICTUS_ROOT%\build" (
  echo "Removing build directory"
  rmdir /Q /S "%INVICTUS_ROOT%\build" || exit /b 7
)

echo "Creating build directory"
mkdir "%INVICTUS_ROOT%\build" || exit /b 8
pushd "%INVICTUS_ROOT%\build"
echo "Spawning cmake generator"
call "%INVICTUS_ROOT%\keyhotee\run_cmake.bat" || exit /b 9
rem /p:VCTargetsPath="C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V110/"
msbuild.exe /M:%NUMBER_OF_PROCESSORS% /p:Configuration=RelWithDebinfo /p:Platform=Win32 /target:rebuild /clp:ErrorsOnly keyhotee.sln /flp:logfile=autobuild_keyhotee_release.txt 1>> build.log 2>&1
if %ERRORLEVEL% neq 0 exit /b 10

rem to be in startup dir
popd 

