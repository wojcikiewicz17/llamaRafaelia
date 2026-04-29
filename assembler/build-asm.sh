#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${ROOT_DIR}/dist"
mkdir -p "${OUT_DIR}"

build_x86_64() {
  as --64 "${ROOT_DIR}/src/hello_x86_64.S" -o "${OUT_DIR}/hello_x86_64.o"
  ld -m elf_x86_64 -e _start "${OUT_DIR}/hello_x86_64.o" -o "${OUT_DIR}/hello_x86_64"
  rm -f "${OUT_DIR}/hello_x86_64.o"
}

build_arm64() {
  clang -target aarch64-linux-gnu -nostdlib -fno-pie -no-pie -fuse-ld=lld -Wl,-e,_start \
    "${ROOT_DIR}/src/hello_arm64.S" -o "${OUT_DIR}/hello_arm64"
}

build_armv7() {
  clang -target armv7a-linux-gnueabihf -nostdlib -fno-pie -no-pie -fuse-ld=lld -Wl,-e,_start \
    "${ROOT_DIR}/src/hello_armv7.S" -o "${OUT_DIR}/hello_armv7"
}

build_x86_64
build_arm64
build_armv7

ls -lh "${OUT_DIR}"/hello_*
