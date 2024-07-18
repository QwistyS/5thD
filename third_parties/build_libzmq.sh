#!/bin/bash

set -e

# Ensure libsodium is installed and available
if ! pkg-config --exists libsodium; then
    echo "libsodium is not installed or not found by pkg-config."
    exit 1
fi

ZMQ_DIR="$PWD/libzmq"
BUILD_DIR="$PWD/build/zmq"
INSTALL_DIR="$PWD/../../lib"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$ZMQ_DIR" \
    -DCMAKE_INSTALL_PREFIX="$BUILD_DIR" \
    -DWITH_PERF_TOOL=OFF \
    -DZMQ_BUILD_TESTS=OFF \
    -DWITH_LIBSODIUM=ON \
    -DENABLE_CURVE=ON

make
make install

cp "$BUILD_DIR/lib/libzmq.so" "$INSTALL_DIR"
