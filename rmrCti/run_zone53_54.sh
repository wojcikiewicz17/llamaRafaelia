#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

F="$HOME/storage/downloads/conversations.json"

Z53_OFF=444596224
Z54_OFF=452984832
BYTES=1048576

echo "[+] file: $F"
ls -lh "$F"
python3 - <<PY
import os
p=os.path.expanduser("$F")
print("size_bytes=", os.path.getsize(p))
PY

echo
echo "[+] slicing..."
./omega_slice "$F" "$Z53_OFF" "$BYTES" > z53.bin
./omega_slice "$F" "$Z54_OFF" "$BYTES" > z54.bin

echo
echo "[+] sizes:"
ls -lh z53.bin z54.bin || true
wc -c z53.bin z54.bin || true

# se vier 0, aborta com diagnóstico
if [ ! -s z53.bin ] || [ ! -s z54.bin ]; then
  echo
  echo "[!] slice veio vazio. Teste direto (64 bytes) p/ ver erro:"
  echo "    ./omega_slice \"$F\" $Z54_OFF 64 | xxd -g 1"
  ./omega_slice "$F" "$Z54_OFF" 64 | xxd -g 1 || true
  exit 2
fi

echo
echo "[+] quick NEON counters"
echo "== z53 =="
./omega_neon < z53.bin
echo "== z54 =="
./omega_neon < z54.bin

echo
echo "[+] hexdump head"
echo "== z53 =="
xxd -g 1 -l 256 z53.bin | head
echo "== z54 =="
xxd -g 1 -l 256 z54.bin | head

echo
echo "[OK] done."
