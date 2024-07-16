#!/bin/bash

set -e

# Define directory variables
SPDLOG_DIR="$PWD/spdlog"
BUILD_DIR="$PWD/build/spdlog"
INSTALL_DIR="$PWD/../../lib"

cd "$SPDLOG_DIR"

# Configure the build with CMake
cmake -S . -B "$BUILD_DIR" -DSPDLOG_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX="$BUILD_DIR"

# Build and install the library
cmake --build "$BUILD_DIR" --target install

# Verify the built library
if [ -f "$BUILD_DIR/lib/libspdlog.so" ]; then
    echo "Library built successfully: $BUILD_DIR/lib/libspdlog.so"
else
    echo "Error: libspdlog.so not found in $BUILD_DIR/lib"
    exit 1
fi

# Copy the shared library to the install directory
cp "$BUILD_DIR/lib/libspdlog.so" "$INSTALL_DIR"

# Verify the copied library
if [ -f "$INSTALL_DIR/libspdlog.so" ]; then
    echo "Library copied successfully to: $INSTALL_DIR/libspdlog.so"
else
    echo "Error: Failed to copy libspdlog.so to $INSTALL_DIR"
    exit 1
fi
