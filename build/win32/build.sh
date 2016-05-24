#!/bin/bash -e

cd `dirname $0`

BUILDROOT="build/win32"

cd ../..

VERSION=$1
REVISION=$2
FOLDER="tbftss-$1"
OUT="$BUILDROOT/$FOLDER"

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
cp CHANGELOG $OUT
cp README.md $OUT

cp /usr/x86_64-w64-mingw32/bin/*.dll $OUT

cd $BUILDROOT

zip -r tbftss-${VERSION}-${REVISION}.win32.zip $FOLDER

mv *.zip ../../dist

rm -rf $FOLDER
