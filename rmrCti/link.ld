/* link.ld - layout simples AArch64 */
ENTRY(_start)

SECTIONS
{
  . = 0x40000000; /* endereço “conveniente” p/ QEMU virt (ajustável) */

  .text : ALIGN(16) {
    KEEP(*(.text.start))
    *(.text*)
  }

  .rodata : ALIGN(16) { *(.rodata*) *(.rodata.*) }

  .data : ALIGN(16) { *(.data*) }

  __bss_start__ = .;
  .bss : ALIGN(16) { *(.bss*) *(COMMON) }
  __bss_end__ = .;

  /* stack */
  . = ALIGN(16);
  . += 0x10000;        /* 64 KiB stack */
  _stack_top = .;
}
