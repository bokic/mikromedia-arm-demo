#!/usr/bin/env python3
"""
Computes and patches the NXP LPC2000 bootloader vector checksum.
In the LPC ARM7TDMI exception vector table, word 5 (offset 0x14)
is reserved such that the sum of the first 8 32-bit vector words equals 0.
"""

import struct
import sys

def patch_checksum(bin_path):
    with open(bin_path, "r+b") as f:
        data = bytearray(f.read())
        if len(data) < 32:
            sys.stderr.write(f"Error: {bin_path} is smaller than 32 bytes\n")
            sys.exit(1)

        # Clear vector 5 slot (reserved slot at 0x14) before sum
        struct.pack_into("<I", data, 0x14, 0)
        words = [struct.unpack_from("<I", data, i * 4)[0] for i in range(8)]
        total = sum(words[:5] + words[6:8]) & 0xFFFFFFFF
        checksum = (0x100000000 - total) & 0xFFFFFFFF

        struct.pack_into("<I", data, 0x14, checksum)
        f.seek(0)
        f.write(data)
        print(f"Patched LPC checksum 0x{checksum:08X} at offset 0x14 in {bin_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write(f"Usage: {sys.argv[0]} <binary_file>\n")
        sys.exit(1)
    patch_checksum(sys.argv[1])
