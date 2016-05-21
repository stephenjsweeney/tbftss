#!/bin/bash -e

cd `dirname $0`

BUILDROOT="build/linux"

cd ../..

VERSION=$1
REVISION=$2
SIZE=0

make clean
make
make dist

rm -rf tbftss
