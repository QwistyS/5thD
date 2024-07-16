#!/bin/bash

set -e

SQLCIPHER_DIR="$PWD/sqlcipher"
BUILD_DIR="$PWD/build/sqlcipher"

cd "$SQLCIPHER_DIR"

./configure \
    --prefix="$BUILD_DIR" \
    --enable-tempstore=yes \
    CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS5"

make
make install

cp "$BUILD_DIR/lib/libsqlcipher.so" "cp "$PWD/../../lib/"
