#!/bin/bash
pushd `dirname $0`/.. > /dev/null
INVICTUS_ROOT=`pwd -P`
popd > /dev/null

export INVICTUS_ROOT
echo "Using: "$INVICTUS_ROOT "as INVICTUS_ROOT"

export QTDIR=$INVICTUS_ROOT/QT
echo "Using: "$QTDIR "as QTDIR"

export DBROOTDIR=$INVICTUS_ROOT/BerkeleyDB
echo "Using: "$DBROOTDIR "as DBROOTDIR"
