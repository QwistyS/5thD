#!/bin/bash

set -e

ZMQ_DIR="$PWD/libzmq"
BUILD_DIR="$PWD/build/zmq"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$ZMQ_DIR" \
    -DCMAKE_INSTALL_PREFIX="$BUILD_DIR" \
    -DWITH_PERF_TOOL=OFF \
    -DZMQ_BUILD_TESTS=OFF \
    -DWITH_LIBSODIUM=ON

make
make install

cp "$BUILD_DIR/lib/libzmq.so" "$PWD/../../lib/"
