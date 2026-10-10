cmake_minimum_required(VERSION 3.15)

# Optional default toolchain file before project()
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake"
        CACHE FILEPATH "Default ARM toolchain file")
endif()

project(mikromedia-demo C ASM)

# Ensure C standard is C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the build type" FORCE)
endif()

# Processor architecture flags
set(CPU_FLAGS "-mcpu=arm7tdmi -marm")

# Common compiler flags
set(COMMON_C_FLAGS
    "${CPU_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -fno-builtin -ffunction-sections -fdata-sections"
)

set(CMAKE_C_FLAGS "${COMMON_C_FLAGS}" CACHE STRING "C compiler flags" FORCE)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG" CACHE STRING "C Debug flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0 -DNDEBUG" CACHE STRING "C Release flags" FORCE)

# Assembler flags (use C preprocessor for .s assembly files)
set(COMMON_ASM_FLAGS "${CPU_FLAGS} -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${COMMON_ASM_FLAGS}" CACHE STRING "ASM compiler flags" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG "-DDEBUG" CACHE STRING "ASM Debug flags" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "ASM Release flags" FORCE)

# Linker script
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lpc2148.ld")

# Common link flags
set(LINK_FLAGS
    "${CPU_FLAGS} -T\"${LINKER_SCRIPT}\" -nostartfiles -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map"
)
set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "Linker flags" FORCE)

# Include directories
include_directories(
    src
    src/lpc214x
    src/fs
    src/fs/fat
)

# Project sources
set(SOURCES
    src/accel.c
    src/crp.c
    src/display.c
    src/font.c
    src/i2c.c
    src/main.c
    src/mmc.c
    src/mp3.c
    src/spi.c
    src/touchscreen.c
    src/utils.c
    src/lpc214x/serial.c
    src/lpc214x/spi.c
    src/lpc214x/utility.c
    src/fs/fs.c
    src/fs/fat/fat_access.c
    src/fs/fat/fat_cache.c
    src/fs/fat/fat_filelib.c
    src/fs/fat/fat_format.c
    src/fs/fat/fat_misc.c
    src/fs/fat/fat_string.c
    src/fs/fat/fat_table.c
    src/fs/fat/fat_write.c
    src/cr_startup_lpc21.s
)

# Set compile definitions for startup & legacy code
add_compile_definitions(
    USE_OLD_STYLE_DATA_BSS_INIT
)

# Executable target (.elf)
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Explicit dependencies on linker script
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    LINK_DEPENDS "${LINKER_SCRIPT}"
)

# Post-build targets: generate .hex, .bin, calculate checksum, and print size
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lpc_checksum.py" "${PROJECT_NAME}.bin"
    COMMENT "Generating Intel HEX, binary and patching LPC2000 bootloader checksum..."
)
