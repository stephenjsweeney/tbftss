#!/bin/bash -e

cd `dirname $0`

BUILDROOT="build/linux"
OUT="$BUILDROOT/tbftss"

cd ../..

VERSION=$1
REVISION=$2
SIZE=0

make clean
make
make dist

mkdir -p $OUT
rm -rf $OUT/*

mkdir -p $OUT/DEBIAN

mkdir -p $OUT/usr/bin
cp tbftss $OUT/usr/bin

mkdir -p $OUT/opt/tbftss
cp -r data $OUT/opt/tbftss
cp -r gfx $OUT/opt/tbftss
cp -r music $OUT/opt/tbftss
cp -r sound $OUT/opt/tbftss
cp -r manual $OUT/opt/tbftss

SIZE=`du -bs $OUT | cut -f -1`
SIZE=`expr $SIZE / 1024`

sed \
-e "s/\${version}/$VERSION-$REVISION/" \
-e "s/\${size}/$SIZE/" \
$BUILDROOT/control > $OUT/DEBIAN/control

cd $BUILDROOT

dpkg-deb --build tbftss

mv tbftss.deb tbftss-${VERSION}-${REVISION}.i386.deb
alien -r --bump=0 tbftss-${VERSION}-${REVISION}.i386.deb

mv *.deb *.rpm ../../dist

rm -rf tbftss
