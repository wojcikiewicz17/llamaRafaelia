#!/usr/bin/env python3
import json
IN="omega_threads.jsonl"
OUT="omega_threads_rank.txt"

rows=[]
with open(IN,"r",encoding="utf-8") as f:
  for line in f:
    r=json.loads(line)
    rows.append(r)

# rank: depth desc, msgs desc, bytes desc
rows.sort(key=lambda r:(r.get("depth",0), r.get("msgs",0), r.get("bytes",0)), reverse=True)

with open(OUT,"w",encoding="utf-8") as o:
  o.write("rank\tdepth\tmsgs\tbytes\ttool_pct\tconv_i\troot\tmodels\n")
  for i,r in enumerate(rows[:50],1):
    models=",".join(r.get("models",[])) if isinstance(r.get("models"),list) else ""
    o.write(f"{i}\t{r.get('depth',0)}\t{r.get('msgs',0)}\t{r.get('bytes',0)}\t{r.get('tool_pct',0)}\t{r.get('conv_i',-1)}\t{r.get('root','')}\t{models}\n")

print("[B] OK ->", OUT)
