Windows
=======
#### Prerequisites ####
* Microsoft Visual C++ 2013 Update 2 (the free Express edition will work)

#### Set up the directory structure####
* Create a base directory for all projects.  I'm putting everything in 
  `E:\Invictus`, you can use whatever you like.  In several of the batch files 
  and makefiles, this directory will be referred to as `INVICTUS_ROOT`:
  ```
mkdir E:\Invictus
```

* Clone the Keyhotee repository
  ```
cd E:\Invictus
git clone https://github.com/InvictusInnovations/keyhotee.git
cd keyhotee
git clone https://github.com/InvictusInnovations/BitShares.git
cd ..
git clone https://github.com/bitshares/bitshares_toolkit.git
cd bitshares_toolkit
git submodule init
git submodule update
cd vendor
git clone https://github.com/InvictusInnovations/leveldb-win.git
```

* Dowload CMake
  
  Download the latest *Win32 Zip* build CMake from 
  http://cmake.org/cmake/resources/software.html (version 2.8.12.2 as of this 
  writing).  Unzip it to your base directory, which will create a directory that
  looks something like `E:\Invictus\cmake-2.8.12.2-win32-x86`.  Rename this 
  directory to `E:\Invictus\CMake`.

  If you already have CMake installed elsewhere on your system you can use it, 
  but Keyhotee has a few batch files that expect it to be in the base 
  directory's `CMake` subdirectory, so those scripts would need tweaking.

* Download library dependencies:
 * ICU and QT
   
   Keyhotee depends on ICU4C version 52.1 and Qt 5.2.  You can build these from
   source or download our pre-built binaries to speed things up.  To download 
   the pre-built versions:
   * download http://get.syncad.com/invictus/static_ICU_QT5.2.zip
   * unzip it to the base directory `E:\Invictus`
 * BerkeleyDB

   Keyhotee depends on BerkeleyDB 12c Release 1 (12.1.6.0.20).  You can build 
   this from source or download our pre-built binaries to speed things up.  To 
   download the pre-built versions:
   * download http://get.syncad.com/invictus/BerkeleyDB.zip
   * unzip it to the base directory `E:\Invictus`

   If you prefer to build your own version of ICU, there is the script 
   [https://github.com/InvictusInnovations/keyhotee/blob/master/build_icu.bat build_icu.bat] 
   containing all commands to build it from scratch.  Likewise for QT, there is
   a script [https://github.com/InvictusInnovations/keyhotee/blob/master/build_qt.bat build_qt.bat] 
   to build it.  For both libraries, we build static versions to cut down on the
   number of DLLs we need to ship.

 * Boost
 
   Keyhotee depends on the Boost libraries version 1.53 or later (I assume 
   you're using 1.55, the latest as of this writing).  You must build them from
   source.
   * download the latest boost source from http://www.boost.org/users/download/
   * unzip it to the base directory `E:\Invictus`. 
   * This will create a directory like `E:\Invictus\boost_1_55_0`. Rename this 
   directory to `E:\Invictus\boost`
 * OpenSSL

   Keyhotee depends on OpenSSL, and you must build this from source.
    * download the latest OpenSSL source from http://www.openssl.org/source/
    * Untar it to the base directory `E:\Invictus`
    * this will create a directory like `E:\Invictus\openssl-1.0.1f`.

    I believe you could also download a pre-built version of OpenSSL.  If you 
	want to try this, I'd recommend you:
    * download one of the installers from http://slproweb.com/products/Win32OpenSSL.html
    * run it and tell it to install to `E:\Invictus\OpenSSL`

At the end of this, your base directory should look like this:
```
E:\Invictus
+- BerkeleyDB
+- bitshares_toolkit
+- boost
+- CMake
+- ICU
+- keyhotee
+- openssl-1.0.1f
+- QT
```
If you're using pre-built OpenSSL, you will have the directory `OpenSSL` 
instead of `openssl-1.0.1f`.  
#### Build the library dependencies ####

* Set up environment for building:
  ```
cd E:\Invictus\keyhotee
setenv.bat
```

* Build boost libraries:
  ```
cd E:\Invictus\boost
bootstrap.bat
b2.exe link=shared
```
  The file `E:\Invictus\keyhotee\BitShares\fc\CMakeLists.txt` has the 
  `FIND_PACKAGE(Boost ...)`
  command that makes CMake link in Boost.  That file contains the line:
  ```
SET(Boost_USE_DEBUG_PYTHON ON)
```
  Edit this file and comment this line out (with a `#`).
  This line  tells CMake to look for a boost library that was built with 
  `b2.exe link=shared python-debugging=on`.  That would cause debug builds to 
  have `-gyd` mangled into their filename.  We don't need python debugging here,
  so we didn't give the `python-debugging` argument to `b2.exe`, and
  that causes our boost debug libraries to have `-gd` mangled into the filename 
  instead.  If this option in `fc\CMakeLists.txt` doesn't match the way you 
  compiled boost, CMake won't be able to find the debug version of the boost 
  libraries, and you'll get some strange errors when you try to run the
  debug version of Keyhotee.

* Build OpenSSL DLLs
  ```
cd E:\Invictus\openssl-1.0.1f
perl Configure --openssldir=E:\Invictus\OpenSSL VC-WIN32
ms\do_ms.bat
nmake -f ms\ntdll.mak
nmake -f ms\ntdll.mak install
```
  This will create the directory `E:\Invictus\OpenSSL` with the libraries, DLLs,
  and header files.

#### Build project files for Keyhotee ####

* Run CMake:
  ```
cd E:\Invcitus\keyhotee
run_cmake.bat
```
 This pops up the cmake gui, but if you've used CMake before it will probably be
 showing the wrong data, so fix that:
 * Where is the source code: `E:\Invictus\keyhotee`
 * Where to build the binaries: `E:\Invictus\vs12`
 
 Then hit **Configure**.  It may ask you to specify a generator for this 
 project; if it does, choose **Visual Studio 12** and select **Use default 
 native compilers**.  Look through the output and fix any errors.  Then 
 hit **Generate**.

#### Build Keyhotee ####
* Launch *Visual Studio* and load `E:\Invictus\vs12\keyhotee.sln`
* set *Keyhotee* as the startup project
* *Build Solution*

 This will build Keyhotee.exe (KeyhoteeD.exe for debug) to `E:\Invictus\bin`

#### Copy DLLs ####
You'll need to copy boost, OpenSSL, and ICU DLLs to the `bin` directory
so Keyhotee can find them at runtime.  You can do this manually:
```
cd E:\Invictus
copy OpenSSL\bin\libeay.dll bin
copy boost\stage\lib\boost_chrono-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_chrono-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_context-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_context-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_coroutine-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_coroutine-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_date_time-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_date_time-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_filesystem-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_filesystem-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_system-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_system-vc110-mt-1_55.dll bin
copy boost\stage\lib\boost_thread-vc110-mt-gd-1_55.dll bin
copy boost\stage\lib\boost_thread-vc110-mt-1_55.dll bin
```

Or you can build the `INSTALL` target in Visual Studio which will
copy all of the necessary files into your `E:\Invictus\install`
directory, then copy all of those files to the `bin` directory.
