#! /bin/sh

export QTDIR=/home/syncad/InvictusRoot/QT
export PATH=$QTDIR/bin:$PATH
export SQLITE3SRCDIR=$QTDIR/src/3rdparty/sqlite

# Install all deps described on http://trac.webkit.org/wiki/BuildingQtOnLinux
# Additionally following libs must be installed:
#- libwebp-dev
#- libxcomposite-dev
#- libxslt-dev
#- libXcomposite-dev

# Install this patch before building https://bugreports.qt-project.org/browse/QTBUG-35444?page=com.atlassian.jira.plugin.system.issuetabpanels:changehistory-tabpanel

../src/configure -static -no-c++11 -skip qtwebkit -no-sse3 -no-avx -no-avx2 -no-ssse3 -no-sse4.1 -no-sse4.2 -prefix $QTDIR -nomake examples -nomake tests -opensource  -confirm-license -debug-and-release -force-debug-info -qt-sql-sqlite -qt-zlib -qt-libpng -qt-libjpeg -qt-xcb -qt-freetype -qt-pcre -qt-xkbcommon 2>&1 | tee configure.log
#make 2>&1 | tee build.log 
#make install 2>&1 | tee install.log

