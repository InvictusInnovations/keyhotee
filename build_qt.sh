#! /bin/sh

export QTDIR=/home/syncad/InvictusRoot/QT
export PATH=$QTDIR/bin:$PATH
export SQLITE3SRCDIR=$QTDIR/src/3rdparty/sqlite

# Install all deps described on http://trac.webkit.org/wiki/BuildingQtOnLinux
# Additionally following libs have to been installed:
#- libwebp-dev
#- libxcomposite-dev
#- libxslt-dev
#- libXcomposite-dev

# Install this patch before building https://bugreports.qt-project.org/browse/QTBUG-35444?page=com.atlassian.jira.plugin.system.issuetabpanels:changehistory-tabpanel

QT_COMMON_OPTIONS="-static -no-c++11 -skip qtwebkit -no-opengl -no-sse3 -no-avx -no-avx2 -no-ssse3 -no-sse4.1 -no-sse4.2 -no-compile-examples -nomake examples -nomake tests -opensource -confirm-license -qt-sql-sqlite -qt-zlib -qt-libpng -qt-libjpeg -qt-xcb -qt-pcre -qt-xkbcommon"
#-qt-freetype
echo $QT_COMMON_OPTIONS
PROC_COUNT=`grep -c processor /proc/cpuinfo`
echo $PROC_COUNT

../src/configure -release -force-debug-info -prefix $QTDIR $QT_COMMON_OPTIONS 2>&1 | tee $QTDIR/configure_release.log && \
make -j $PROC_COUNT 2>&1 | tee $QTDIR/build_release.log && make install 2>&1 | tee $QTDIR/install_release.log && \
cd .. && \
rm -rf $QTDIR/build && \
mkdir build && \
cd build && \
../src/configure -debug -prefix $QTDIR/debug $QT_COMMON_OPTIONS 2>&1 | tee $QTDIR/configure_debug.log && \
make -j $PROC_COUNT 2>&1 | tee $QTDIR/build_debug.log && make install 2>&1 | tee $QTDIR/install_debug.log

