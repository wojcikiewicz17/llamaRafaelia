#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

F="${1:-zone_timeline.txt}"

echo "[info] file=$F"
echo "[info] lines=$(wc -l < "$F" || true)"
echo

echo "== zone counts (top 20) =="
awk '
  {tot++; z[$2]++}
  END{
    for(k in z){
      printf "%s\t%d\t%.6f%%\n", k, z[k], 100.0*z[k]/tot
    }
  }
' "$F" | sort -k2,2nr | head -n 20

echo
echo "== check whitespace vs TAB (first 5 raw lines) =="
sed -n '1,5p' "$F" | cat -A

echo
echo "== zone0 count (whitespace split) =="
awk '$2==0{c++} END{print c+0}' "$F"
