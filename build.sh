#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CONFIGURATION="${1:-Debug}"

case "$CONFIGURATION" in
    Debug|Release) ;;
    *)
        echo "Usage: $0 [Debug|Release]" >&2
        exit 2
        ;;
esac

BUILD_DIR="$ROOT_DIR/build-$CONFIGURATION"

cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$CONFIGURATION" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT_DIR/cmake/arm-none-eabi.cmake"

exec cmake --build "$BUILD_DIR"

