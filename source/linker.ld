ENTRY(start)

PHDRS
{
    text PT_LOAD FLAGS (r + x);
    rodata PT_LOAD FLAGS (r);
    data PT_LOAD FLAGS (r + w);
}

SECTIONS
{
    . = 0xFF001000;

    .text ALIGN(4K) : {
        *(.start) /* source/kernel/x86/kernel_32.asm */
        *(.text)
        *(.text.*)
    } :text

    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    } :rodata

    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } :data
}