#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
TL="${1:-zone_timeline.txt}"
TOP="${2:-20}"

# zone -> total msgs, convs únicas, min/max conv_i
awk -F'\t' '
  {z=$2; c=$3; tot[z]++; seen[z,c]=1;
   if(!(z in min) || c<min[z]) min[z]=c;
   if(!(z in max) || c>max[z]) max[z]=c;
  }
  END{
    for(z in tot){
      u=0;
      for(k in seen){
        split(k,a,SUBSEP);
        if(a[1]==z) u++;
      }
      printf "%d\t%d\t%d\t%d\t%d\n", z, tot[z], u, min[z], max[z];
    }
  }
' "$TL" | sort -k2,2nr | head -n "$TOP" \
| awk 'BEGIN{print "zone\ttotal\tconvs_unique\tconv_min\tconv_max"} {print}'
