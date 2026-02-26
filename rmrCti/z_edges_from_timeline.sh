#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
F="${1:-zone_timeline.txt}"
N="${2:-60}"

awk '
  NR==1{last=$2; next}
  {cur=$2; e=last "->" cur; c[e]++; last=cur}
  END{
    for(k in c) print c[k] "\t" k
  }
' "$F" | sort -k1,1nr | head -n "$N" \
| awk '{printf "EDGE\t%s\t%s\n",$2,$1}'
