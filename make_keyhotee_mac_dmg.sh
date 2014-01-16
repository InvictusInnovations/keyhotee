#!/bin/sh

# prerequisites: you need to have brew (http://brew.sh) installed
#
# run once: brew tap homebrew/versions && brew install cmake qt5 gcc48
#
# you also need to download the source tarball for ICU and Boost and put
# them somewhere on your system

# Set the following variables to the ones of your system:
export GCC_BIN=/usr/local/bin/gcc-4.8
export GXX_BIN=/usr/local/bin/g++-4.8
export ICU_TARBALL=/tmp/icu4c-52_1-src.tgz
export BOOST_TARBALL=/tmp/boost_1_55_0.tar.bz2
export QTDIR=/usr/local/Cellar/qt5/5.2.0

BUILD_DIR="/tmp/keyhotee_build"
mkdir -p $BUILD_DIR


# compile icu
cd $BUILD_DIR
tar xf $ICU_TARBALL && cd icu/source

U_USING_ICU_NAMESPACE=0 U_CHARSET_IS_UTF8=1 ./runConfigureICU MacOSX/GCC --enable-static --disable-shared CC=$GCC_BIN CXX=$GXX_BIN
make -j4 && make install DESTDIR=$BUILD_DIR


# compile boost
cd $BUILD_DIR
tar xf $BOOST_TARBALL && cd boost_1_55_0

# trick boost to use our version of gcc
ln -s $GCC_BIN $BUILD_DIR/gcc
ln -s $GXX_BIN $BUILD_DIR/g++

PATH=$BUILD_DIR:$PATH ./bootstrap.sh --with-toolset=gcc --prefix=$BUILD_DIR/usr/local --with-libraries=thread,date_time,system,filesystem,program_options,signals,serialization,chrono,context,coroutine,test
PATH=$BUILD_DIR:$PATH ./b2 threading=multi link=static
PATH=$BUILD_DIR:$PATH ./b2 install threading=multi link=static


# compile keyhotee

cd $BUILD_DIR
git clone https://github.com/InvictusInnovations/keyhotee
cd keyhotee
git clone https://github.com/InvictusInnovations/BitShares
cd BitShares
git clone https://github.com/InvictusInnovations/fc
cd ..

cmake -DCMAKE_CXX_COMPILER=$GXX_BIN -DCMAKE_C_COMPILER=$GCC_BIN -DCMAKE_LIBRARY_PATH=$BUILD_DIR/usr/local/lib  -DCMAKE_INCLUDE_PATH=$BUILD_DIR/usr/local/include -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" ./CMakeLists.txt
#make VERBOSE=1
make -j4

cd bin
$QTDIR/bin/macdeployqt Keyhotee.app -dmg
