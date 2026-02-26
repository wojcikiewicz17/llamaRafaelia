#!/usr/bin/env sh
set -e

F="$HOME/storage/downloads/conversations.json"
EVENTS="omega_events.jsonl"
OUT="regime.index.jsonl"
CHUNK=$((8*1024*1024))   # 8MB

echo "[OMEGA] arquivo: $F"
ls -lh "$F"

echo "[OMEGA] lendo $EVENTS"
echo "[OMEGA] saída -> $OUT"
: > "$OUT"

grep '"event":"regime"' "$EVENTS" | while read -r line; do
  ZONE=$(echo "$line" | sed -n 's/.*"zone":\([0-9]*\).*/\1/p')
  OFF=$(echo "$line" | sed -n 's/.*"off":\([0-9]*\).*/\1/p')
  DPPM=$(echo "$line" | sed -n 's/.*"dppm":\([0-9]*\).*/\1/p')

  echo "[ZONE $ZONE] off=$OFF dppm=$DPPM"

  ./omega_slice2 "$F" "$OFF" "$CHUNK" \
    | ./omega_neon \
    | sed "s/^/{\"zone\":$ZONE,\"off\":$OFF,\"dppm\":$DPPM,/" \
    | sed 's/$/}/' \
    >> "$OUT"
done

echo "[OMEGA] DONE"
echo "[OMEGA] $(wc -l < "$OUT") zonas REGIME indexadas"
