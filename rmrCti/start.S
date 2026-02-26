/* start.S (AArch64 baremetal)
   - seta stack
   - zera .bss
   - chama rf_main
*/
  .section .text.start, "ax"
  .global _start
  .type _start, %function

_start:
  /* stack = _stack_top */
  ldr x0, =_stack_top
  mov sp, x0

  /* zero .bss: [__bss_start__, __bss_end__) */
  ldr x1, =__bss_start__
  ldr x2, =__bss_end__
  cmp x1, x2
  b.ge 2f
1:
  str xzr, [x1], #8
  cmp x1, x2
  b.lt 1b
2:
  bl rf_main
3:
  wfe
  b 3b
