#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-Debug}"

HEX_FILE="$ROOT_DIR/build-$BUILD_TYPE/mikromedia-demo.hex"
if [[ ! -f "$HEX_FILE" ]]; then
    HEX_FILE="$ROOT_DIR/build/mikromedia-demo.hex"
fi

if [[ ! -f "$HEX_FILE" ]]; then
    echo "Error: Firmware file not found: $HEX_FILE" >&2
    echo "Please build the project first: ./build.sh $BUILD_TYPE" >&2
    exit 1
fi

echo "=========================================================="
echo " Upload via USB HID Bootloader (mikroBootloader)"
echo " Firmware: $HEX_FILE"
echo "=========================================================="
echo ""
echo "Instructions:"
echo "1. Connect mikromedia for ARM via onboard USB cable."
echo "2. Press the RESET button on the board."
echo "3. The onboard bootloader waits ~5 seconds after reset."
echo ""

if command -v mikrobootloader >/dev/null 2>&1; then
    echo "Running mikrobootloader CLI..."
    exec mikrobootloader -f "$HEX_FILE"
elif command -v hid-bootloader-cli >/dev/null 2>&1; then
    echo "Running hid-bootloader-cli..."
    exec hid-bootloader-cli -mmcu=LPC2148 -v "$HEX_FILE"
else
    cat <<EOF
Note: No automated CLI tool ('mikrobootloader' or 'hid-bootloader-cli') was found in PATH.

To flash with GUI:
- Open 'mikroBootloader USB HID' (or 'NECTO Studio' / 'mikroC Bootloader')
- Press RESET on the board and click 'Connect' within 5 seconds.
- Select: $HEX_FILE
- Click 'Start Bootloader'.
EOF
fi
