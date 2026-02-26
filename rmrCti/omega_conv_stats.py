#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json, math, collections

INP = "omega_msgs.jsonl"
OUT = "omega_conv_stats.jsonl"

stats = {}
total_density = []

with open(INP, "r", encoding="utf-8") as f:
    for line in f:
        r = json.loads(line)
        ci = r["conv_i"]
        st = stats.get(ci)
        if not st:
            st = {
                "conv_i": ci,
                "title": r.get("conv_title",""),
                "msgs": 0,
                "tool_msgs": 0,
                "bytes": 0,
                "roles": collections.Counter(),
                "models": collections.Counter()
            }
            stats[ci] = st

        st["msgs"] += 1
        st["bytes"] += int(r.get("text_len",0))
        role = r.get("role","")
        st["roles"][role] += 1

        model = r.get("model","")
        if model:
            st["models"][model] += 1

        if r.get("has_tool"):
            st["tool_msgs"] += 1

# calcular densidade
for st in stats.values():
    d = st["bytes"] / max(1, st["msgs"])
    st["density"] = int(d)
    total_density.append(d)

# estatística global
mean = sum(total_density)/len(total_density)
std = math.sqrt(sum((x-mean)**2 for x in total_density)/len(total_density))
thr = mean + 2*std

# escrever saída
with open(OUT, "w", encoding="utf-8") as fo:
    for st in stats.values():
        st["tool_pct"] = round(100.0*st["tool_msgs"]/max(1,st["msgs"]),2)
        st["hot"] = st["density"] >= thr
        st["roles"] = dict(st["roles"])
        st["models"] = dict(st["models"])
        fo.write(json.dumps(st, ensure_ascii=False) + "\n")

print("[omega_conv_stats] OK")
print("convs:", len(stats))
print("density mean:", round(mean,2), "std:", round(std,2), "hot_thr:", round(thr,2))
print("out:", OUT)
