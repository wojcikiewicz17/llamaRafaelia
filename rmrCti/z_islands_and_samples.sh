#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

TL="${1:-zone_timeline.txt}"
MS="${2:-omega_msgs.jsonl}"
Z="${3:-11}"       # zona alvo
K="${4:-10}"       # qtde exemplos

echo "[info] timeline=$TL"
echo "[info] msgs=$MS"
echo "[info] target_zone=$Z examples=$K"
echo

echo "== first nonzero transitions (first 40) =="
awk '
  NR==1{pz=$2; next}
  $2!=pz{
    print NR-1, pz, "->", $2
    pz=$2
    if(++c>=40) exit
  }
' "$TL"

echo
echo "== first msg_i for zone Z =="
N=$(awk -v Z="$Z" '$2==Z{print $1; exit}' "$TL" || true)
echo "first_msg_i=$N"
if [ -n "${N:-}" ]; then
  echo
  echo "== sample message (head 600 bytes) =="
  sed -n "$((N+1))p" "$MS" | head -c 600; echo
fi

echo
echo "== $K spaced samples from zone Z (head 220 bytes each) =="
awk -v Z="$Z" '$2==Z{print $1}' "$TL" | awk 'NR%70==1{print}' | head -n "$K" \
| while read -r I; do
    echo "---- msg_i=$I ----"
    sed -n "$((I+1))p" "$MS" | head -c 220; echo
  done
