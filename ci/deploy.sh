#!/usr/bin/env bash

export LC_ALL=C.UTF-8

cd ..
cd build-ci/BZX-$BUILD_TARGET/src
mkdir -p bin
ls -lah

ARCHIVE_CMD="zip"

if [[ $BUILD_TARGET = "win64" ]]; then
  ARCHIVE_NAME="win64.zip"
elif [[ $HOST = "x86_64-w64-mingw32" ]]; then
    ARCHIVE_NAME="windows-x64.zip"
elif [[ $HOST = "arm-linux-gnueabihf" ]]; then
    ARCHIVE_NAME="arm-x86.tar.gz"
    ARCHIVE_CMD="tar -czf"
elif [[ $HOST = "aarch64-linux-gnu" ]]; then
    ARCHIVE_NAME="arm-x64.tar.gz"
    ARCHIVE_CMD="tar -czf"
elif [[ $HOST = "x86_64-unknown-linux-gnu" ]]; then
    ARCHIVE_NAME="linux-x64.tar.gz"
    ARCHIVE_CMD="tar -czf"
elif [[ $HOST = "x86_64-apple-darwin14" ]]; then
    ARCHIVE_NAME="osx-x64.zip"
fi

cp qt/zcoin-qt bin/ || cp qt/zcoin-qt.exe bin/ || echo "no qt"
cp bitcoinzerod bin/ || cp bitcoinzerod.exe bin/ || echo "no daemon"
cp bitcoinzero-cli bin/ || cp bitcoinzero-cli.exe bin/ || echo "no cli"
cp bitcoinzero-tx bin/ || cp bitcoinzero-tx.exe bin/ || echo "no tx"
echo "before bin"

cd bin || echo "bin failed"
echo "listing src/bin"
ls -lah
echo "strip files"
strip * || echo "nothing to strip"

ARCHIVE_CMD="$ARCHIVE_CMD $ARCHIVE_NAME *"
eval $ARCHIVE_CMD

mkdir -p zip
ls -lah
mv $ARCHIVE_NAME zip
cd zip
echo "listing src/bin/zip"
ls -lah

sleep $(( ( RANDOM % 6 ) + 1 ))s
