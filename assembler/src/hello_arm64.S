.section .text
.global _start

_start:
  mov x0, #1
  adrp x1, msg
  add x1, x1, :lo12:msg
  mov x2, #msg_len
  mov x8, #64
  svc #0

  mov x0, #0
  mov x8, #93
  svc #0

.section .rodata
msg:
  .ascii "[arm64] rmrCti preserved; assembler active.\n"
.set msg_len, .-msg
