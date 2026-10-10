#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-Debug}"
INTERFACE="${2:-interface/jlink.cfg}"

HEX_FILE="$ROOT_DIR/build-$BUILD_TYPE/mikromedia-demo.hex"
if [[ ! -f "$HEX_FILE" ]]; then
    HEX_FILE="$ROOT_DIR/build/mikromedia-demo.hex"
fi

if [[ ! -f "$HEX_FILE" ]]; then
    echo "Error: Firmware file not found: $HEX_FILE" >&2
    echo "Please build the project first: ./build.sh $BUILD_TYPE" >&2
    exit 1
fi

if ! command -v openocd >/dev/null 2>&1; then
    echo "Error: openocd is not installed or not in PATH." >&2
    echo "Install it via: sudo pacman -S openocd (or apt install openocd)" >&2
    exit 1
fi

echo "=========================================================="
echo " Uploading firmware via OpenOCD"
echo " Interface: $INTERFACE"
echo " Target:    target/lpc2148.cfg"
echo " Firmware:  $HEX_FILE"
echo "=========================================================="

exec openocd \
    -f "$INTERFACE" \
    -f "target/lpc2148.cfg" \
    -c "init" \
    -c "reset halt" \
    -c "flash write_image erase \"$HEX_FILE\"" \
    -c "reset run" \
    -c "shutdown"
