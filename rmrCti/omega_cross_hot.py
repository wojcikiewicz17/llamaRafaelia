#!/usr/bin/env python3
import json, collections

hot=set()
for l in open("omega_hot_convs.jsonl","r",encoding="utf-8"):
    hot.add(json.loads(l)["conv_i"])

stats=collections.defaultdict(lambda:{"msgs":0,"tool":0})
for l in open("omega_msgs.jsonl","r",encoding="utf-8"):
    r=json.loads(l)
    if r["conv_i"] in hot:
        k=r.get("model","") or "unknown"
        stats[k]["msgs"]+=1
        stats[k]["tool"]+=1 if r.get("has_tool") else 0

with open("omega_cross_hot.jsonl","w",encoding="utf-8") as o:
    for k,v in stats.items():
        o.write(json.dumps({
            "model":k,
            "msgs":v["msgs"],
            "tool_pct": round(100*v["tool"]/max(1,v["msgs"]),2)
        },ensure_ascii=False)+"\n")
print("[omega_cross_hot] OK")
