#!/bin/bash -e

BUILDROOT="build/tbftss"

VERSION=0.4
REVISION=`date +"%y%m%d"`
SIZE=0

cd ..

rm -rf dist/*

make clean
make
make dist

mkdir -p $BUILDROOT
rm -rf $BUILDROOT/*

mkdir -p $BUILDROOT/DEBIAN

mkdir -p $BUILDROOT/usr/bin
cp tbftss $BUILDROOT/usr/bin

mkdir -p $BUILDROOT/opt/tbftss
cp -r data $BUILDROOT/opt/tbftss
cp -r gfx $BUILDROOT/opt/tbftss
cp -r music $BUILDROOT/opt/tbftss
cp -r sound $BUILDROOT/opt/tbftss
cp -r manual $BUILDROOT/opt/tbftss

SIZE=`du -bs $BUILDROOT | cut -f -1`
SIZE=`expr $SIZE / 1024`

sed \
-e "s/\${version}/$VERSION-$REVISION/" \
-e "s/\${size}/$SIZE/" \
build/control > $BUILDROOT/DEBIAN/control

cd build

dpkg-deb --build tbftss

mv tbftss.deb tbftss-${VERSION}-${REVISION}.i386.deb
alien -r --bump=0 tbftss-${VERSION}-${REVISION}.i386.deb

mv *.deb *.rpm ../dist

rm -rf tbftss
