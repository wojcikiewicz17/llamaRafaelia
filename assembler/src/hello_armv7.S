.syntax unified
.arm
.section .text
.global _start

_start:
  mov r0, #1
  movw r1, #:lower16:msg
  movt r1, #:upper16:msg
  movw r2, #:lower16:msg_len
  mov r7, #4
  svc #0

  mov r0, #0
  mov r7, #1
  svc #0

.section .rodata
msg:
  .ascii "[armv7] rmrCti preserved; assembler active.\n"
.set msg_len, .-msg
