#!/usr/bin/env python3
import json
IN="omega_conv_stats.jsonl"
OUT="omega_hot_convs.jsonl"
RANK="omega_hot_rank.txt"

hots=[]
with open(IN,"r",encoding="utf-8") as f:
  for line in f:
    r=json.loads(line)
    if r.get("hot"):
      hots.append(r)

# rank por density desc, depois bytes
hots.sort(key=lambda r:(r.get("density",0), r.get("bytes",0)), reverse=True)

with open(OUT,"w",encoding="utf-8") as o:
  for r in hots:
    o.write(json.dumps(r,ensure_ascii=False)+"\n")

with open(RANK,"w",encoding="utf-8") as o:
  o.write("rank\tdensity\tbytes\tmsgs\ttool_pct\tconv_i\ttitle\n")
  for i,r in enumerate(hots[:50],1):
    o.write(f"{i}\t{r.get('density',0)}\t{r.get('bytes',0)}\t{r.get('msgs',0)}\t{r.get('tool_pct',0)}\t{r.get('conv_i',-1)}\t{r.get('title','')}\n")

print("[A] OK ->", OUT, "and", RANK)
