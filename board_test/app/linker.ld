/* STM32F446xC */

MEMORY
{
    rom0 (rx)  : ORIGIN = 0x08000000, LENGTH = 256k
    ram0 (xrw) : ORIGIN = 0x20000000, LENGTH = 128k
}

__ram0_start__           = ORIGIN(ram0);
__ram0_size__            = LENGTH(ram0);
__ram0_end__             = __ram0_start__ + __ram0_size__;

_estack                  = __ram0_end__;

HEAP_SIZE = 0x8000; /* 32kB for heap */

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    } > rom0

    _textdata = .;

    .data :
    {
        _sdata = .;
        *(.data)
        *(.data.*)
        _edata = .;
    } > ram0 AT > rom0

    .bss (NOLOAD):
    {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        _bss_end = .;
    } > ram0

    .heap (NOLOAD):
    {
        . = ALIGN(4);
        _heap_start = .;
        . = . + HEAP_SIZE;
        _heap_end = .;
    } > ram0
}
