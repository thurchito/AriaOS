ENTRY(boot)
SECTIONS {
    . = 0x7c00;
    .boot :
    {
        *(.boot)
    }
    . = 0x8000;
    __kernel_start = .;
    __kernel_start_seg = __kernel_start >> 4;
    .text : AT(0x7e00)
    {
        *(.text.start)
        *(.text*)
    }
    .rodata :
    {
        *(.rodata*)
    }
    .data :
    {
        *(.data)
    }
    __kernel_end = .;
    __kernel_size_sectors = (__kernel_end - __kernel_start + 511) / 512;

    .bss :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        . = ALIGN(4);
        __bss_end = .;
        __bss_sizel = (__bss_end - __bss_start) / 4;
    }
}