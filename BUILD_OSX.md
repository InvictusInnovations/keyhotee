* Install XCode from the App Store

  From a terminal, run `xcode-select --install` to install the command-line 
  tools *(probably unnecessary)*
* Install Qt from http://qt-project.org/downloads

  I used the *Qt Online Installer for Mac*.  During installation, I installed 
  to the default location (`$HOME/Qt`) and selected the *clang 64-bit* files 
  for *Qt 5.2.1* and the *Tools*, deselecting all other platforms.
* Download the OpenSSL source from http://www.openssl.org/source/ and untar it 
  in your base directory
  ```
./Configure darwin64-x86_64-cc --openssldir=$HOME/Invictus
make
make install
```

* Build ICU
  * Download ICU4C from the usual location 
  * Untar it in `$HOME/Invictus`, creating an `icu` directory
    ```
cd $HOME/Invictus/icu
mkdir build
cd build
../source/runConfigure MacOSX --prefix=$HOME/Invictus/icu --enable-static --disable-shared
gnumake
gnumake install
```
* Install boost
  ```
cd $HOME/Invictus
tar xvjf boost_1_55_0.tar.bz2
mv boost_1_55_0 boost
./bootstrap.sh
./b2
```

* Install Berkeley DB
  ```
cd $HOME/Invictus
tar xvzf db-60.30.tar.gz
cd db-6.0.30/build_unix
../dist/configure --prefix=$HOME/Invictus/BerkeleyDB --enable-cxx
make
make install
```

* Install CMake

  I used the *Mac OSX 64/32-bit Universal* `.dmg` installer and installed to 
  the default location, and installed the command-line links.
* Make a base directory for your source and clone the Keyhotee repositories:
  ```
mkdir $HOME/Invictus
cd $HOME/Invictus
git clone https://github.com/InvictusInnovations/keyhotee.git
cd keyhotee
git clone https://github.com/InvictusInnovations/BitShares.git
cd BitShares
git clone https://github.com/InvictusInnovations/fc.git
```

* Run CMake

  * Let CMake know where to find our libraries:
    ```
export CMAKE_PREFIX_PATH=$HOME/Invictus/OpenSSL:$HOME/Invictus/icu:$HOME/Invictus/BerkeleyDB:$HOME/Qt/5.2.1/clang_64
export DYLD_FRAMEWORK_PATH=$HOME/Qt/5.2.1/clang_64/lib:$DYLD_FRAMEWORK_PATH
```

  * run `cmake-gui`
  * select *Unix Makefiles*
  * hit *Configure*, fix any errors, then hit *Generate*
  * run `make`
  * launch Keyhotee via `open bin/Keyhotee.app`
