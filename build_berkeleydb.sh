#! /bin/sh

# Simple script to compile a BerkeleyDB in a way needed by KH project.
# BerkeleyDb sources should be unpacked into any directory (BerkeleyDB_BuildRoot). $INVICTUS_ROOT/BerkeleyDB will be used as install prefix.
# then this script should be spawned from directory $BerkeleyDB_BuildRoot/build_unix

PROC_COUNT=`grep -c processor /proc/cpuinfo`
echo $PROC_COUNT

../dist/configure --prefix=$INVICTUS_ROOT/BerkeleyDB --enable-cxx --enable-shared=no 2>&1 | tee configure.log &&\
make -j $PROC_COUNT 2>&1 | tee build.log && \
make install | tee install.log


