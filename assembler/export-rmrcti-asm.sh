#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${ROOT_DIR}/rmrCti"
OUT_DIR="${ROOT_DIR}/assembler/rmrCti_x86_64"
ERR_DIR="${OUT_DIR}/errors"
LOG_OK="${OUT_DIR}/compiled.txt"
LOG_FAIL="${OUT_DIR}/failed.txt"
LOG_CMD="${OUT_DIR}/compile_commands.txt"

mkdir -p "${OUT_DIR}" "${ERR_DIR}"
: > "${LOG_OK}"
: > "${LOG_FAIL}"
: > "${LOG_CMD}"

try_compile() {
  local src="$1" out="$2" err="$3"

  if cc -S -O2 -std=gnu11 -D_GNU_SOURCE "$src" -o "$out" >"$err" 2>&1; then
    echo "cc -S -O2 -std=gnu11 -D_GNU_SOURCE"; return 0
  fi

  if clang -S -O2 -std=gnu11 -D_GNU_SOURCE "$src" -o "$out" >"$err" 2>&1; then
    echo "clang -S -O2 -std=gnu11 -D_GNU_SOURCE"; return 0
  fi

  if cc -S -O0 -std=gnu11 -D_GNU_SOURCE -Wno-implicit-function-declaration "$src" -o "$out" >"$err" 2>&1; then
    echo "cc -S -O0 -std=gnu11 -D_GNU_SOURCE -Wno-implicit-function-declaration"; return 0
  fi

  return 1
}

shopt -s nullglob
for src in "${SRC_DIR}"/*.c; do
  base="$(basename "${src}" .c)"
  out="${OUT_DIR}/${base}.s"
  err="${ERR_DIR}/${base}.log"

  if cmd_used="$(try_compile "${src}" "${out}" "${err}")"; then
    echo "${base}.c" >> "${LOG_OK}"
    echo "${base}.c :: ${cmd_used}" >> "${LOG_CMD}"
    rm -f "${err}"
  else
    rm -f "${out}"
    echo "${base}.c" >> "${LOG_FAIL}"
  fi
done

ok_count=$(wc -l < "${LOG_OK}" | tr -d ' ')
fail_count=$(wc -l < "${LOG_FAIL}" | tr -d ' ')

echo "rmrCti -> assembly x86_64: ok=${ok_count} fail=${fail_count}"
if [[ "${fail_count}" -gt 0 ]]; then
  echo "Falharam (veja ${LOG_FAIL} e ${ERR_DIR}/):"
  cat "${LOG_FAIL}"
fi
