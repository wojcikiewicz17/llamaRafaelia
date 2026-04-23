.section .text
.global _start

_start:
  mov $1, %rax
  mov $1, %rdi
  lea msg(%rip), %rsi
  mov $msg_len, %rdx
  syscall

  mov $60, %rax
  xor %rdi, %rdi
  syscall

.section .rodata
msg:
  .ascii "[x86_64] rmrCti preserved; assembler active.\n"
.set msg_len, .-msg
