#!/bin/bash -xe
if [ -e "BitShares" ]
then
    cd BitShares
    git pull
else
    git clone https://github.com/InvictusInnovations/BitShares.git BitShares
    cd BitShares
fi

if [ -e "fc" ]
then
    cd fc
    git pull
    cd ..
else
    git clone https://github.com/InvictusInnovations/fc.git fc
fi

cd $WORKSPACE

if [ -e "bitshares_toolkit" ]
then
    cd bitshares_toolkit
    git pull
else
    git clone https://github.com/BitShares/bitshares_toolkit.git bitshares_toolkit
    cd bitshares_toolkit
    git submodule init
    git submodule update
fi

cd vendor

if [ -e "leveldb-win/.git" ]
then
    cd leveldb-win
    git pull
    cd ..
else
    git clone https://github.com/InvictusInnovations/leveldb-win.git
fi

cd $WORKSPACE

rm -rf build
mkdir build
cd build

export INVICTUS_ROOT=$WORKSPACE

. ../keyhotee/setenv.sh

cmake -DBUILD_VERSION_PATCH=$BUILD_NUMBER -DCMAKE_TOOLCHAIN_FILE=$INVICTUS_ROOT/toolchain.invictus/toolchain.invictus.cmake ../keyhotee
make -j8

# Lets put /usr/bin at the begining of PATH
# to use system-wide ldd for cpack
# instead of toolchain.invictus version
PATH=/usr/bin:$PATH cpack --verbose
