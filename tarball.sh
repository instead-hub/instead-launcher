#!/usr/bin/env bash
VERSION="0.6.3"
VERTITLE="instead-launcher-$VERSION"
ARCHIVE="instead-launcher_$VERSION.tar.gz"
qmake-qt4
make -C unzip clean
make clean
git ls-files --others --exclude-standard | while read l; do rm -rf $l; done
ln -sf ./ "$VERTITLE"
tar -cz --exclude "$VERTITLE/$VERTITLE" --exclude .git --exclude tarball.sh --exclude "$ARCHIVE" -f "$ARCHIVE" "$VERTITLE"/*
rm -f "$VERTITLE"
