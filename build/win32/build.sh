#!/bin/bash -e

cd `dirname $0`

BUILDROOT="build/win32"
OUT="$BUILDROOT/tbftss"

cd ../..

VERSION=$1
REVISION=$2
SIZE=0

make -f makefile.win32 clean
make -f makefile.win32

mkdir -p $OUT
rm -rf $OUT/*

cp tbftss.exe $OUT

cp -r data $OUT
cp -r gfx $OUT
cp -r music $OUT
cp -r sound $OUT
cp -r manual $OUT
cp -r locale $OUT
cp LICENSE $OUT
cp README.md $OUT

cp /usr/x86_64-w64-mingw32/bin/*.dll $OUT

cd $BUILDROOT

zip -r tbftss-${VERSION}-${REVISION}.win32.zip tbftss

mv *.zip ../../dist

rm -rf tbftss
