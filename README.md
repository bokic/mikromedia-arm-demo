# mikromedia for ARM (LPC2148) Demo

Demonstration and board support package for the **mikromedia for ARM** development board by [MikroElektronika (MIKROE)](https://www.mikroe.com/).

![mikromedia for ARM](docs/mikromedia-arm.jpg)

---

## Hardware Overview

- **Board Name**: mikromedia for ARM (LPC2148)
- **Product ID / PID**: MIKROE-780
- **Product Reference**: [TME MIKROE-780 Catalog Entry](https://www.tme.eu/en/details/mikroe-780/other-development-tools/mikroelektronika/)
- **Manufacturer**: MikroElektronika (MIKROE)
- **Lifecycle Status**: **Discontinued / Legacy (End-of-Life)**. The legacy ARM7TDMI-S series has been superseded by newer ARM Cortex-M based mikromedia boards.
- **Microcontroller**: NXP **LPC2148**
  - **Core**: ARM7TDMI-S (32-bit RISC) @ up to 60 MHz
  - **Flash**: 512 KB (500 KB usable for user code, top sectors reserved for bootloader/IAP)
  - **SRAM**: 32 KB internal RAM + 8 KB USB RAM
- **Onboard Peripherals**:
  - **Display**: 320x240 TFT color display with resistive touch panel (`HX8347` / compatible driver)
  - **Audio Codec**: VS1053 MP3 / audio decoder with 3.5 mm stereo audio jack
  - **Accelerometer**: Analog Devices ADXL345 3-axis accelerometer (I2C)
  - **Storage**: MicroSD / MMC card slot (SPI) with FAT16/FAT32 file system support
  - **USB**: Full-speed USB 2.0 interface
  - **Power Management**: Li-Polymer battery connector with onboard charging circuit

---

## Project Structure

```
├── CMakeLists.txt              # Top-level CMake build configuration
├── build.sh                    # Helper script for building Debug/Release
├── cmake/
│   ├── arm-none-eabi.cmake     # CMake cross-compilation toolchain file
│   ├── lpc2148.ld              # GNU linker script for NXP LPC2148
│   └── lpc_checksum.py         # Utility to compute and patch NXP LPC2000 bootloader vector checksum
└── src/
    ├── main.c                  # Application entry point and demonstration flow
    ├── cr_startup_lpc21.s      # Startup assembly code and vector table
    ├── crp.c                   # Code Read Protection configuration
    ├── display.[ch]            # TFT display driver
    ├── font.[ch]               # Font rendering and glyphs
    ├── touchscreen.[ch]        # Resistive touch screen driver and calibration
    ├── accel.[ch]              # ADXL345 accelerometer driver (I2C)
    ├── mp3.[ch]                # VS1053 MP3 codec driver (SPI)
    ├── mmc.[ch]                # SD/MMC low-level block I/O (SPI)
    ├── utils.[ch]              # Clock configuration and timing utilities
    ├── fs/                     # FAT file system integration
    └── lpc214x/                # LPC2148 register definitions and peripheral drivers (UART, SPI, etc.)
```

---

## Prerequisites

To build the firmware, the following tools are required:

- **CMake** (version 3.15 or newer)
- **GNU Arm Embedded Toolchain** (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`)
- **Python 3** (used during post-build to inject the LPC bootloader checksum)
- **Make** or **Ninja**

On Arch Linux:
```bash
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib cmake python make
```

On Debian / Ubuntu:
```bash
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi cmake python3 make
```

---

## Building the Firmware

Both CMake and helper scripts default to the **Debug** configuration.

### Quick Build

```bash
# Build Debug configuration (default)
./build.sh

# Or explicitly specify configuration:
./build.sh Debug
./build.sh Release
```

### Using CMake Directly

```bash
# Configure (defaults to Debug)
cmake -B build-Debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake

# Compile
cmake --build build-Debug
```

The build generates three output artifacts in `build-<Config>/`:
- `mikromedia-demo.elf`: Executable ELF with debug symbols.
- `mikromedia-demo.hex`: Intel HEX format for bootloader/ISP flash tools.
- `mikromedia-demo.bin`: Raw binary with the LPC2000 interrupt vector checksum automatically patched at offset `0x14`.

---

## Flashing & Running the Firmware

Helper scripts are provided for each upload method. By default, they flash the firmware from `build-Debug/` (or `build/`).

### 1. USB HID Bootloader (Factory Default)

The board ships from the factory with the MikroElektronika USB HID bootloader pre-programmed:

```bash
./build.sh && ./upload_usb.sh
```

**Workflow:**
1. Connect the mikromedia board to your PC via a USB cable.
2. Press the **RESET** button on the board.
3. Run `./upload_usb.sh` within 5 seconds of resetting (the bootloader waits 5 seconds before jumping to user code).
4. If `mikrobootloader` or `hid-bootloader-cli` is found in `PATH`, it uploads automatically.
5. Alternatively, open the **mikroBootloader USB HID** desktop GUI, press RESET, click **Connect**, select `build-Debug/mikromedia-demo.hex`, and click **Start Bootloader**.

---

### 2. OpenOCD JTAG

If you are using a JTAG hardware debugger (e.g., Olimex, J-Link, FTDI):

```bash
./upload_openocd.sh [Debug|Release] [interface_config]
```

Example with custom interface probe:
```bash
OPENOCD_INTERFACE=interface/ftdi/olimex-arm-usb-ocd-h.cfg ./upload_openocd.sh Debug
```

---

### 3. Segger J-Link

If connecting a Segger J-Link or J-Link EDU probe:

```bash
./upload_jlink.sh [Debug|Release] [target_speed_khz]
```

Example:
```bash
./upload_jlink.sh Debug
```

---

### 4. UART0 ISP (Serial Bootloader)

The LPC2148 includes an on-chip ROM bootloader over UART0 (pins P0.0 / P0.1):

```bash
./upload_uart.sh [serial_port] [Debug|Release] [baud_rate]
```

Example with USB-to-UART adapter on `/dev/ttyUSB0`:
```bash
./upload_uart.sh /dev/ttyUSB0 Debug 115200
```
*(Requires `lpc214isp` or `lpc21isp` installed)*.

---

## Demo Features on Board

Once uploaded, the board runs the interactive multi-peripheral showcase:
- **TFT Display**: High-speed color primitives and text rendering.
- **ADXL345 Accelerometer**: Real-time tilt spirit level bubble and G-force readings.
- **Touch Screen**: Interactive drawing canvas with calibration and clear controls.
- **MicroSD Card**: SPI card detection and FAT file system initialization status.
- **VS1053 Audio Codec**: Sine wave test audio output via the 3.5mm stereo jack.
