#!/usr/bin/env bash

LOCAL_DIR=$(cd "$(dirname "$0")" && pwd)
PRESET=macos-arm64-debug
BUILD_DIR="$LOCAL_DIR/build/binary/$PRESET"
cmake --preset $PRESET && cmake --build "$BUILD_DIR" --target local-sample && cd $BUILD_DIR/samples/local/ && ./local-sample