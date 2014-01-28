#!/bin/sh

# by following the instructions in this script, you will be able to build
# a clean .dmg for Keyhotee that should be portable to all mac systems without
# any additional dependencies
#
# prerequisite: a fresh mavericks install (in a vm, most likely)
#
# note: this script cannot be run as is, sometimes it will require manual
#       intervention. You should follow the instructions in the lines
#       starting with "#!!! ".

## install xcode and command line tools

#!!! download xcode from the mac app store

xcode-select --install
#!!! install command line tools

sudo xcodebuild -license
#!!! agree to the license terms


## install brew and keyhotee dependencies

ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

brew install cmake boost qt5
export QTDIR=/usr/local/Cellar/qt5/5.2.0


## git clone keyhotee and compile it
git clone https://github.com/InvictusInnovations/keyhotee
cd keyhotee
git clone https://github.com/InvictusInnovations/BitShares
cd BitShares
git clone https://github.com/InvictusInnovations/fc
cd ..

cmake . && make

cd bin
$QTDIR/bin/macdeployqt Keyhotee.app -dmg
