#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

cc -O3 -pipe -Wall -Wextra -std=c11 \
  omega_zone_pipeline_fix.c -o ozp

./ozp omega_zone_metrics.jsonl \
     omega_msgs.jsonl \
     omega_metrics_v3.jsonl

echo
echo "== preview =="
sed -n '1,12p' omega_zone_conv_rank.txt || true
echo
sed -n '1,12p' zone_stats.txt || true
echo
sed -n '1,12p' zone_markov_edges.txt || true
echo
sed -n '1,12p' zone_conv_streaks.txt || true
