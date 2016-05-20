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

make DESTDIR=$OUT install

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
