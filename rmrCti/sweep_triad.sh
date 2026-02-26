#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$ROOT/triad"
ANALYZE="$ROOT/analyze_trace.py"

STEPS="${1:-400}"

# parâmetros base
seed=42
R=2.0
r=0.7
ZCAP=1000000

# grades (as que você já está usando)
AS=(7 9 11)          # alpha = pi/a
BS=(6 9 12)          # beta  = pi/b
KAPPAS=(0.10 0.16 0.22 0.28 0.34)
LAMS=(300 600 1000 1600 2400)

OUT="$ROOT/out_sweep"
RESULTS="$OUT/results.csv"

mkdir -p "$OUT/runs"

# header (sobrescreve)
echo "steps,seed,R,r,alpha,beta,kappa,lam,zcap,rows,esc,stable_rate,p_peaks,p_non,delta,j0,j1,j2,j3" > "$RESULTS"

# sanity
[ -x "$BIN" ] || { echo "FAIL :: BIN not executable: $BIN"; exit 2; }
[ -f "$ANALYZE" ] || { echo "FAIL :: analyze not found: $ANALYZE"; exit 2; }

pi=$(python3 - <<'PY'
import math
print(math.pi)
PY
)

for a in "${AS[@]}"; do
  alpha=$(python3 - <<PY
pi=$pi
a=$a
print(pi/a)
PY
)

  for b in "${BS[@]}"; do
    beta=$(python3 - <<PY
pi=$pi
b=$b
print(pi/b)
PY
)

    for kappa in "${KAPPAS[@]}"; do
      for lam in "${LAMS[@]}"; do
        tag="s${seed}_a${a}_b${b}_k${kappa}_L${lam}"
        run_dir="$OUT/runs/$tag"
        mkdir -p "$run_dir"

        "$BIN" \
          --steps "$STEPS" --seed "$seed" --out "$run_dir" \
          --R "$R" --r "$r" --alpha "$alpha" --beta "$beta" \
          --kappa "$kappa" --lam "$lam" --zcap "$ZCAP" \
          >/dev/null

        line=$(python3 "$ANALYZE" "$run_dir/triad_trace.csv")
        echo "$STEPS,$seed,$R,$r,$alpha,$beta,$kappa,$lam,$ZCAP,$line" >> "$RESULTS"

        # delta é o 6º campo dentro do line (rows,esc,stable_rate,p_peaks,p_non,delta,...)
        echo "OK $tag :: $(echo "$line" | cut -d',' -f6) (ΔP)"
      done
    done

  done
done

echo "DONE :: $RESULTS"
