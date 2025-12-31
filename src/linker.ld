PHDRS
{
  text PT_LOAD FLAGS (5); /* PF_R | PF_X => 4+1 = 5 */
  data PT_LOAD FLAGS (6); /* PF_R | PF_W => 4+2 = 6 */
}

ENTRY(_start)
SECTIONS
{
    . = 0x80000;
    .text : { *(.text.boot) *(.text) } :text
    .rodata : { *(.rodata) } :text

    . = ALIGN(0x8);
    .data : { *(.data) } :data

    bss_begin = .;
    .bss (NOLOAD) : { *(.bss*) } :data
    bss_end = .;
}
