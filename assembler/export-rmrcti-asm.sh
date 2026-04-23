#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${ROOT_DIR}/rmrCti"
OUT_DIR="${ROOT_DIR}/assembler/rmrCti_x86_64"
LOG_OK="${OUT_DIR}/compiled.txt"
LOG_FAIL="${OUT_DIR}/failed.txt"

mkdir -p "${OUT_DIR}"
: > "${LOG_OK}"
: > "${LOG_FAIL}"

shopt -s nullglob
for src in "${SRC_DIR}"/*.c; do
  base="$(basename "${src}" .c)"
  out="${OUT_DIR}/${base}.s"
  if cc -S -O2 -std=c11 "${src}" -o "${out}" >/dev/null 2>&1; then
    echo "${base}.c" >> "${LOG_OK}"
  else
    rm -f "${out}"
    echo "${base}.c" >> "${LOG_FAIL}"
  fi
done

ok_count=$(wc -l < "${LOG_OK}" | tr -d ' ')
fail_count=$(wc -l < "${LOG_FAIL}" | tr -d ' ')

echo "rmrCti -> assembly x86_64: ok=${ok_count} fail=${fail_count}"
if [[ "${fail_count}" -gt 0 ]]; then
  echo "Falharam (veja ${LOG_FAIL}):"
  cat "${LOG_FAIL}"
fi
