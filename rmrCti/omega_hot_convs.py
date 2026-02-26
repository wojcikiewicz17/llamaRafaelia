#!/usr/bin/env python3
import json
IN="omega_conv_stats.jsonl"
OUT="omega_hot_convs.jsonl"
with open(IN,"r",encoding="utf-8") as f, open(OUT,"w",encoding="utf-8") as o:
    for line in f:
        r=json.loads(line)
        if r.get("hot"):
            o.write(json.dumps(r,ensure_ascii=False)+"\n")
print("[omega_hot_convs] OK")
