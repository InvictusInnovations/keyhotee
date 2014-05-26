#! /bin/sh -e

export INVICTUS_ROOT=/home/syncad/InvictusRoot

export TOOLCHAIN_ROOT=$INVICTUS_ROOT/toolchain.invictus

ICU_ROOT=$INVICTUS_ROOT/ICU
export ICU_ROOT

OPENSSL_ROOT=$INVICTUS_ROOT/openssl
export OPENSSL_ROOT

export QTDIR=$INVICTUS_ROOT/QT
PATH=$QTDIR/bin:$TOOLCHAIN_ROOT/bin:$PATH
export PATH
export SQLITE3SRCDIR=$QTDIR/src/3rdparty/sqlite

LD_LIBRARY_PATH=$ICU_ROOT/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

export PKG_CONFIG_SYSROOT_DIR=$TOOLCHAIN_ROOT/x86_64-unknown-linux-gnu/sysroot
export PKG_CONFIG_PATH=$TOOLCHAIN_ROOT/x86_64-unknown-linux-gnu/sysroot/usr/lib/pkgconfig

which gcc
which g++

# Install all deps described on http://trac.webkit.org/wiki/BuildingQtOnLinux
# Additionally following libs have to been installed:
#- libwebp-dev
#- libxcomposite-dev
#- libxslt-dev
#- libXcomposite-dev

# Install this patch before building https://bugreports.qt-project.org/browse/QTBUG-35444?page=com.atlassian.jira.plugin.system.issuetabpanels:changehistory-tabpanel

# https://bugreports.qt-project.org/browse/QTBUG-19565
sed -e "s@#include <linux/futex.h>@#define FUTEX_WAIT 0\n#define FUTEX_WAKE 1@g" -i ../src/qtbase/src/corelib/thread/qmutex_linux.cpp

#Copy our one mkspecs to setup all needed options
cp -rf $INVICTUS_ROOT/keyhotee/HelperScripts/qt-mkspecs/linux-syncad-g++ $QTDIR/src/qtbase/mkspecs/

QT_COMMON_OPTIONS="-xplatform linux-syncad-g++ -sysroot $PKG_CONFIG_SYSROOT_DIR -qtlibinfix Invictus -no-c++11 -no-sse3 -no-avx -no-avx2 -no-ssse3 -no-sse4.1 -no-sse4.2 -no-compile-examples -nomake examples -nomake tests -opensource -confirm-license -qt-sql-sqlite  -qt-libpng -qt-libjpeg -qt-pcre -qt-xcb -qt-xkbcommon -qt-freetype"

# -qt-zlib 
# 

echo $QT_COMMON_OPTIONS
PROC_COUNT=`grep -c processor /proc/cpuinfo`
echo $PROC_COUNT

# -force-debug-info removed since webkit build fails because libWebCore.a exceeds size limits

../src/configure -v -I $OPENSSL_ROOT/include -L $OPENSSL_ROOT/lib -I $ICU_ROOT/include -L $ICU_ROOT/lib -release -extprefix $QTDIR $QT_COMMON_OPTIONS 2>&1 | tee $QTDIR/configure_release.log && \
make -j $PROC_COUNT 2>&1 | tee $QTDIR/build_release.log && make install 2>&1 | tee $QTDIR/install_release.log

