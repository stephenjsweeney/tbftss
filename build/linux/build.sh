#!/bin/bash -e

cd `dirname $0`

BUILDROOT="build/linux"

cd ../..

VERSION=$1
REVISION=$2
SIZE=0

make clean
make src-dist

make clean
make LOCALE_DIR=locale
make dist

rm -rf tbftss
