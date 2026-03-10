#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

ZONE_METRICS=${1:-omega_zone_metrics.jsonl}
MSGS=${2:-omega_msgs.jsonl}
METRICS_V3=${3:-omega_metrics_v3.jsonl}

if [ ! -f "$MSGS" ]; then
  SRC_OBJS="omega_objs.jsonl"
  if [ ! -f "$SRC_OBJS" ]; then
    echo "[erro] faltou $MSGS e também não existe $SRC_OBJS para extração." >&2
    echo "[erro] gere/forneça omega_objs.jsonl ou passe um arquivo de msgs já pronto no 2º argumento." >&2
    exit 1
  fi

  echo "[info] $MSGS não encontrado; extraindo via omega_extract_msgs.py a partir de $SRC_OBJS"
  python3 omega_extract_msgs.py --in_objs "$SRC_OBJS" --out "$MSGS"
fi

cc -O3 -pipe -Wall -Wextra -std=c11 \
  omega_zone_pipeline_fix.c -o ozp

./ozp "$ZONE_METRICS" \
     "$MSGS" \
     "$METRICS_V3"

echo
echo "== preview =="
sed -n '1,12p' omega_zone_conv_rank.txt || true
echo
sed -n '1,12p' zone_stats.txt || true
echo
sed -n '1,12p' zone_markov_edges.txt || true
echo
sed -n '1,12p' zone_conv_streaks.txt || true
