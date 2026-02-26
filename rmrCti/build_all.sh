#!/usr/bin/env bash
set -euo pipefail

echo "==[A] build baremetal =="
./build.sh

echo
echo "==[B] hosted demo =="
clang -O2 -Wall -Wextra hosted_demo.c rf_core.c -o hosted_demo
./hosted_demo

echo
echo "==[C] nm/undef sanity =="
echo "-- rf_core.o undefined (deve vazio):"
nm -u rf_core.o || true

echo "-- exports:"
nm -g --defined-only rf_core.o | sort | head -n 20

echo
echo "==[D] readelf (amostra) =="
readelf -s rf_core.o | grep -nE "rafa_(boot|run)" || true
echo "DONE"
