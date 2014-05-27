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

rm -rf build
mkdir build
cd build

export INVICTUS_ROOT=$WORKSPACE

. ../keyhotee/setenv.sh

cmake -DBUILD_VERSION_PATCH=$BUILD_NUMBER -DCMAKE_TOOLCHAIN_FILE=$INVICTUS_ROOT/toolchain.invictus/toolchain.invictus.cmake ../keyhotee
make -j8

# Adding alias to use system-wide ldd for cpack
# instead of toolchain.invictus version
alias ldd=/usr/bin/ldd
cpack --verbose
unalias ldd

