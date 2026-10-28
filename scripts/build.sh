#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="Release"

if [ "$1" == "debug" ]; then
    BUILD_TYPE="Debug"
fi

echo "=== YüzGizle Derleniyor ($BUILD_TYPE) ==="

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTS=ON
cmake --build "$BUILD_DIR" -j$(nproc)

echo "=== Birim Testler Çalıştırılıyor ==="
./"$BUILD_DIR"/bin/test_core

echo "=== Derleme Tamamlandı: $BUILD_DIR/bin/YuzGizle ==="
