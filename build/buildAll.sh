#!/bin/bash -e

VERSION=`egrep 'VERSION = ([0-9.+])' ../common.mk | awk '{print $3}'`
REVISION=`date +"%y%m%d"`

mkdir -p ../dist
rm -rf ../dist/*

cd build

linux/build.sh $VERSION $REVISION

win32/build.sh $VERSION $REVISION
