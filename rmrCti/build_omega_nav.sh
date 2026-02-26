#!/usr/bin/env bash
set -e

CC="${CC:-cc}"
CFLAGS="-O3 -pipe -Wall -Wextra"
SRC="omega_nav.c"
OUT="omega_nav"

try_link () {
  echo "[build] $CC $CFLAGS $SRC -o $OUT $*"
  $CC $CFLAGS "$SRC" -o "$OUT" "$@" >/dev/null 2>&1
}

# prefer ncursesw (Termux costuma usar wide)
if try_link -lncursesw; then
  echo "[build] OK (ncursesw)"
elif try_link -lncurses; then
  echo "[build] OK (ncurses)"
else
  echo "[build] FAIL: sem ncurses. No Termux:"
  echo "  pkg install ncurses"
  exit 2
fi

chmod +x "$OUT"
echo "[build] binary: ./$OUT"
