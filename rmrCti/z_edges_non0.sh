#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

F="${1:-zone_markov_edges.txt}"
N="${2:-80}"

echo "[info] file=$F"
echo

echo "== edges excluding any 0 (top by count) =="
awk '
  $1=="EDGE"{
    e=$2; c=$3+0
    if(e=="0->0") next
    if(e ~ /^0->/) next
    if(e ~ /->0$/) next
    print c "\t" e
  }
' "$F" | sort -k1,1nr | head -n "$N" | awk '{printf "EDGE\t%s\t%s\n",$2,$1}'
