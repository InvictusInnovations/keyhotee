#!/bin/bash
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

export BOOST_ROOT=/opt/boost_1_55_0
#export OPENSSL_ROOT=/opt/openssl
export QTDIR=/opt/QT
export DBROOTDIR=/opt/BerkeleyDB
export ICUROOT=/opt/ICU
#export PATH=/opt/toolchain/bin:$PATH

#'compat' version of toolchain should be used eventually instead of native
#cmake -DCMAKE_TOOLCHAIN_FILE=/opt/toolchain/toolchain.cmake ../keyhotee
cmake ../keyhotee
make -j8


cd $WORKSPACE/keyhotee/bin
strip Keyhotee
tar -czf Keyhotee.tar.gz Keyhotee


#
# deb packages - currently disabled
#
#remove old packages
#touch dummy.deb
#rm *.deb
#package keyhote
#mkdir -p debian/usr/bin
#mkdir -p debian/DEBIAN
#FILE=Keyhotee
#SIZE=`stat -c %s $WORKSPACE/keyhotee/bin/$FILE` 
#DATE=`date +%Y%m%d`
#DEPS="libboost1.54-all-dev"
#TITLE="Invictus Innovations Keyhotee client"
#TEXT="Client for Invictus Innovations Keyhotee cryptoservice"
#NAME=keyhotee
#
#echo "Package: $NAME
#Version: $DATE-nightly
#Architecture: amd64
#Maintainer: Invictus Innovations <contact.us@invictus-innovations.com>
#Installed-Size: $SIZE
#Depends: $DEPS
#Section: office
#Priority: optional
#Homepage: http://invictus.io
#Description: $TITLE
# $TEXT
#" > debian/DEBIAN/control
#
#cp $WORKSPACE/keyhotee/bin/$FILE debian/usr/bin
#strip debian/usr/bin/$FILE
#chmod 0755 -R debian
#
#fakeroot dpkg-deb -b debian ../$NAME-$DATE-nightly.deb
#
#cd $WORKSPACE/keyhotee/BitShares/bts_wallet

#package bts_wallet
#mkdir -p debian/usr/bin
#mkdir -p debian/DEBIAN
#
#FILE=bts_wallet
#SIZE=`stat -c %s $WORKSPACE/keyhotee/bin/$FILE` 
#DATE=`date +%Y%m%d`
#DEPS="libboost1.54-all-dev"
#TITLE="Invictus Innovations BitShares wallet"
#TEXT="Wallet-Client for Invictus Innovations BitShares cryptocurrency"
#NAME=bts-wallet
#
#echo "Package: $NAME
#Version: $DATE-nightly
#Architecture: amd64
#Maintainer: Invictus Innovations <contact.us@invictus-innovations.com>
#Installed-Size: $SIZE
#Depends: $DEPS
#Section: office
#Priority: optional
#Homepage: http://invictus.io
#Description: $TITLE
# $TEXT
#" > debian/DEBIAN/control
#
#cp $WORKSPACE/keyhotee/bin/$FILE debian/usr/bin
#strip debian/usr/bin/$FILE
#chmod 0755 -R debian
#
#fakeroot dpkg-deb -b debian ../../$NAME-$DATE-nightly.deb
