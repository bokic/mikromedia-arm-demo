#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-Debug}"
SPEED="${2:-4000}"

HEX_FILE="$ROOT_DIR/build-$BUILD_TYPE/mikromedia-demo.hex"
if [[ ! -f "$HEX_FILE" ]]; then
    HEX_FILE="$ROOT_DIR/build/mikromedia-demo.hex"
fi

if [[ ! -f "$HEX_FILE" ]]; then
    echo "Error: Firmware file not found: $HEX_FILE" >&2
    echo "Please build the project first: ./build.sh $BUILD_TYPE" >&2
    exit 1
fi

if ! command -v JLinkExe >/dev/null 2>&1; then
    echo "Error: JLinkExe is not installed or not in PATH." >&2
    exit 1
fi

echo "=========================================================="
echo " Uploading firmware via Segger J-Link"
echo " Device:   LPC2148"
echo " Speed:    ${SPEED} kHz"
echo " Firmware: $HEX_FILE"
echo "=========================================================="

JLINK_CMD_FILE="$(mktemp /tmp/jlink_flash_XXXXXX.jlink)"
cat <<EOF > "$JLINK_CMD_FILE"
device LPC2148
si JTAG
speed $SPEED
r
h
loadfile "$HEX_FILE"
r
g
exit
EOF

trap 'rm -f "$JLINK_CMD_FILE"' EXIT

exec JLinkExe -CommandFile "$JLINK_CMD_FILE"
