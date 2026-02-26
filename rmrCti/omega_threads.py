#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json, collections, argparse, math

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--in_msgs", default="omega_msgs.jsonl")
    ap.add_argument("--out", default="omega_threads.jsonl")
    args=ap.parse_args()

    by_conv = collections.defaultdict(list)
    by_id = {}
    children = collections.defaultdict(list)

    # carregar msgs
    with open(args.in_msgs,"r",encoding="utf-8") as f:
        for line in f:
            r=json.loads(line)
            by_conv[r["conv_i"]].append(r)
            by_id[r["msg_id"]] = r
            if r.get("parent"):
                children[r["parent"]].append(r["msg_id"])

    with open(args.out,"w",encoding="utf-8") as fo:
        for conv_i, msgs in by_conv.items():
            roots=[m["msg_id"] for m in msgs if not m.get("parent")]
            for root in roots:
                stack=[(root,1)]
                depth=0; cnt=0; bytes_=0; tool=0
                roles=[]; models=set()
                while stack:
                    mid,d=stack.pop()
                    m=by_id.get(mid)
                    if not m: continue
                    depth=max(depth,d); cnt+=1
                    bytes_+=m.get("text_len",0)
                    tool+=1 if m.get("has_tool") else 0
                    roles.append(m.get("role",""))
                    if m.get("model"): models.add(m["model"])
                    for ch in children.get(mid,[]):
                        stack.append((ch,d+1))
                fo.write(json.dumps({
                    "conv_i":conv_i,"root":root,"depth":depth,
                    "msgs":cnt,"bytes":bytes_,
                    "tool_pct": round(100*tool/max(1,cnt),2),
                    "roles_path":roles[:10],
                    "models":sorted(models),
                },ensure_ascii=False)+"\n")
    print("[omega_threads] OK")

if __name__=="__main__": main()
