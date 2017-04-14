#!/bin/bash
P=`pwd`
cd ../src
qmake -config release
make -j5
macdeployqt MultiDownload.app/ -dmg -appstore-compliant
cp MultiDownload.dmg $P
cd $P
ls -la MultiDownload.dmg
