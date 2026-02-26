#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
F="${1:-zone_timeline.txt}"
TOP="${2:-40}"

awk '
  {tot++; z[$2]++}
  END{
    for(k in z){
      printf "%d\t%d\t%.6f%%\n", k, z[k], 100.0*z[k]/tot
    }
  }
' "$F" | sort -k2,2nr | head -n "$TOP" \
| awk 'BEGIN{print "zone\trefs\tshare"} {print $1"\t"$2"\t"$3}'
