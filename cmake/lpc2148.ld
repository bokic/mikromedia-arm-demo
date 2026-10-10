/* Linker script for NXP LPC2148 (ARM7TDMI) */

MEMORY
{
    /* Flash ROM: LPC2148 has 512KB total (0x00000000 - 0x0007FFFF)
       Top sectors (0x7D000 - 0x7FFFF) are reserved for IAP / bootloader.
       Available application flash is 500KB (0x00000000 - 0x0007CFFF). */
    FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 500K

    /* Internal SRAM: 32KB (0x40000000 - 0x40007FFF)
       Top 32 bytes reserved for IAP by Philips/NXP ROM */
    RAM (rwx)       : ORIGIN = 0x40000000, LENGTH = 32K - 32

    /* USB RAM: 8KB (0x7FD00000 - 0x7FD01FFF) */
    USB_RAM (rwx)   : ORIGIN = 0x7FD00000, LENGTH = 8K
}

/* Initial top of stack pointer used by cr_startup_lpc21.s */
_vStackTop = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(_boot)

SECTIONS
{
    /* Code and constant data stored in FLASH */
    .text :
    {
        . = ALIGN(4);
        _text = .;
        KEEP(*(.isr_vector))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* ARM exception handling tables */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* Initialized data placed in RAM, loaded from FLASH */
    .data : AT (_etext)
    {
        . = ALIGN(4);
        _data = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    /* Uninitialized data in RAM, cleared to zero by startup code */
    .bss :
    {
        . = ALIGN(4);
        _bss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Heap / memory remaining for runtime allocation */
    .heap (NOLOAD) :
    {
        . = ALIGN(4);
        __end__ = .;
        PROVIDE(end = .);
        *(.heap*)
        . = ALIGN(4);
        __HeapLimit = .;
    } > RAM

    /* Secondary USB RAM section */
    .usb_ram (NOLOAD) :
    {
        . = ALIGN(4);
        *(.usb_ram*)
        . = ALIGN(4);
    } > USB_RAM

    /* Discard compiler debug / unnecessary tables if not needed */
    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
