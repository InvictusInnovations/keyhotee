#!/bin/bash
pushd `dirname $0` > /dev/null
BASE_PATH=`pwd -P`
popd > /dev/null
. $BASE_PATH/setenv.sh
cmake-gui

