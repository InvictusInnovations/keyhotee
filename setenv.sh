#!/bin/bash
pushd `dirname $0`/.. > /dev/null
: ${INVICTUS_ROOT:=`pwd -P`}
popd > /dev/null

export INVICTUS_ROOT
echo "Using: "$INVICTUS_ROOT "as INVICTUS_ROOT"
export BITSHARES_ROOT=$INVICTUS_ROOT

export TOOLCHAIN_ROOT=$INVICTUS_ROOT/toolchain.invictus
export PKG_CONFIG_SYSROOT_DIR=$TOOLCHAIN_ROOT/x86_64-unknown-linux-gnu/sysroot
export PKG_CONFIG_PATH=$TOOLCHAIN_ROOT/x86_64-unknown-linux-gnu/sysroot/usr/lib/pkgconfig

export QTDIR=$INVICTUS_ROOT/QT
echo "Using: "$QTDIR "as QTDIR"

export PATH=$TOOLCHAIN_ROOT/bin:$QTDIR/bin:$INVICTUS_ROOT/bin:$PATH

export OPENSSL_ROOT=$INVICTUS_ROOT/openssl
export OPENSSL_ROOT_DIR=$OPENSSL_ROOT
#export OPENSSL_INCLUDE_DIR=$OPENSSL_ROOT/include
export ICUROOT=$INVICTUS_ROOT/ICU

export LD_LIBRARY_PATH=$ICUROOT/lib:$QTDIR/lib:$LD_LIBRARY_PATH

export DBROOTDIR=$INVICTUS_ROOT/BerkeleyDB
echo "Using: "$DBROOTDIR "as DBROOTDIR"

export BOOST_ROOT=$INVICTUS_ROOT/boost_1_55_0

