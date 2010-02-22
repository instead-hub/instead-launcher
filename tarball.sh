#!/bin/bash
VERSION="0.1"
VERTITLE="instead-launcher-$VERSION"
ARCHIVE="instead-launcher_$VERSION.tar.gz"
svn st | grep "^?" | awk '{ print $$2 }' | grep -v "tarball.sh" | while read l; do rm -rf $$l; done
ln -sf ./ "$VERTITLE"
tar -cz --exclude "$VERTITLE/$VERTITLE" --exclude .svn --exclude "$ARCHIVE" -f "$ARCHIVE" "$VERTITLE"/*
rm -f "$VERTITLE"
