#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

Z="${1:?zip required}"
echo "[info] zip=$Z"
echo

echo "== list (top 60) =="
unzip -l "$Z" | head -n 60

echo
echo "== find likely json/jsonl/md inside =="
unzip -l "$Z" | awk 'tolower($0) ~ /\.(json|jsonl|md|txt)$/ {print}' | head -n 60
