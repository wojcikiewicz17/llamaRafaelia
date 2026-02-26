#!/usr/bin/env python3
import json
IN="omega_cross_hot.jsonl"
OUT="omega_cross_hot_rank.txt"

rows=[]
with open(IN,"r",encoding="utf-8") as f:
  for line in f:
    r=json.loads(line)
    rows.append(r)

# rank por msgs desc
rows.sort(key=lambda r:r.get("msgs",0), reverse=True)

with open(OUT,"w",encoding="utf-8") as o:
  o.write("rank\tmsgs\ttool_pct\tmodel\n")
  for i,r in enumerate(rows,1):
    o.write(f"{i}\t{r.get('msgs',0)}\t{r.get('tool_pct',0)}\t{r.get('model','')}\n")

print("[C] OK ->", OUT)
