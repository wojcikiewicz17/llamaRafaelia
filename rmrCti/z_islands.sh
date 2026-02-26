#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
TL="${1:-zone_timeline.txt}"
Z="${2:-11}"
LIMIT="${3:-40}"

echo "== transitions (first $LIMIT) =="
awk -v LIM="$LIMIT" '
  NR==1{pz=$2; next}
  $2!=pz{
    print NR-1, pz, "->", $2
    pz=$2
    if(++c>=LIM) exit
  }
' "$TL"

echo
echo "== first msg_i for zone=$Z =="
awk -v Z="$Z" '$2==Z{print $1; exit}' "$TL"
