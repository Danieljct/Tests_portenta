/* Linker script para M4 */
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08100000, LENGTH = 1M
    RAM   (xrw)     : ORIGIN = 0x30000000, LENGTH = 288K
    RAM_D1 (xrw)    : ORIGIN = 0x24000000, LENGTH = 512K
    RAM_D2 (xrw)    : ORIGIN = 0x30020000, LENGTH = 288K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
    } > FLASH

    .data :
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > RAM

    .bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > RAM
}
