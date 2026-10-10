#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PORT="${1:-/dev/ttyUSB0}"
BUILD_TYPE="${2:-Debug}"
BAUD="${3:-115200}"
XTAL_KHZ="${4:-12000}"

HEX_FILE="$ROOT_DIR/build-$BUILD_TYPE/mikromedia-demo.hex"
if [[ ! -f "$HEX_FILE" ]]; then
    HEX_FILE="$ROOT_DIR/build/mikromedia-demo.hex"
fi

if [[ ! -f "$HEX_FILE" ]]; then
    echo "Error: Firmware file not found: $HEX_FILE" >&2
    echo "Please build the project first: ./build.sh $BUILD_TYPE" >&2
    exit 1
fi

if ! command -v lpc21isp >/dev/null 2>&1; then
    echo "Error: 'lpc21isp' is not installed or not in PATH." >&2
    echo "Please install lpc21isp (e.g. from your package manager or AUR/source)." >&2
    exit 1
fi

if [[ ! -e "$PORT" ]]; then
    echo "Error: Serial port '$PORT' not found." >&2
    echo "Usage: $0 [port] [BuildType] [baud] [osc_khz]" >&2
    echo "Example: $0 /dev/ttyUSB0 Release 115200 12000" >&2
    exit 1
fi

echo "=========================================================="
echo " Uploading firmware via NXP UART0 ISP"
echo " Port:     $PORT"
echo " Baud:     $BAUD"
echo " Crystal:  ${XTAL_KHZ} kHz"
echo " Firmware: $HEX_FILE"
echo "=========================================================="
echo ""
echo "Reminder: Ensure P0.14 is pulled LOW while resetting the board"
echo "to activate the internal LPC ROM bootloader."
echo ""

exec lpc21isp -hex -control -term "$HEX_FILE" "$PORT" "$BAUD" "$XTAL_KHZ"
