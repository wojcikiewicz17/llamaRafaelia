#!/usr/bin/env bash
set -euo pipefail

# Requisitos: clang + ld.lld + llvm-objcopy (geralmente vêm no Termux via llvm)
# Se você tiver aarch64-none-elf-* toolchain, dá pra adaptar fácil.

CC=${CC:-clang}
LD=${LD:-ld.lld}
OBJCOPY=${OBJCOPY:-llvm-objcopy}

CFLAGS=(
  -O2 -Wall -Wextra
  -ffreestanding -fno-builtin -fno-stack-protector
  -fno-builtin-memcpy -fno-builtin-memset
  -fPIC
  --target=aarch64-none-elf
)

LDFLAGS=(
  -nostdlib
  -T link.ld
)

echo "[1/3] compile..."
$CC "${CFLAGS[@]}" -c rf_core.c  -o rf_core.o
$CC "${CFLAGS[@]}" -c rf_entry.c -o rf_entry.o
$CC "${CFLAGS[@]}" -c start.S   -o start.o

echo "[2/3] link..."
$LD "${LDFLAGS[@]}" start.o rf_core.o rf_entry.o -o rafa_bare.elf

echo "[3/3] bin..."
$OBJCOPY -O binary rafa_bare.elf rafa_bare.bin

echo "OK:"
file rafa_bare.elf
ls -lh rafa_bare.elf rafa_bare.bin
