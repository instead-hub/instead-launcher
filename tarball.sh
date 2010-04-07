#!/bin/bash
VERSION="0.3"
VERTITLE="instead-launcher-$VERSION"
ARCHIVE="instead-launcher_$VERSION.tar.gz"
qmake
make -C unzip clean
make clean
svn st | grep "^?" | awk '{ print $$2 }' | while read l; do rm -rf $l; done
ln -sf ./ "$VERTITLE"
tar -cz --exclude "$VERTITLE/$VERTITLE" --exclude .svn --exclude tarball.sh --exclude "$ARCHIVE" -f "$ARCHIVE" "$VERTITLE"/*
rm -f "$VERTITLE"
