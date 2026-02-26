#!/usr/bin/env python3
import json, sys, argparse

def norm(s): return (s or "").lower()

ap=argparse.ArgumentParser()
ap.add_argument("--term", required=True)
ap.add_argument("--limit", type=int, default=20)
ap.add_argument("--role", default="")
ap.add_argument("--in", dest="inp", default="omega_msgs.jsonl")
args=ap.parse_args()

term=norm(args.term)
want_role=norm(args.role)
lim=max(1,args.limit)

hits=0
with open(args.inp,"r",encoding="utf-8",buffering=1024*1024) as f:
  for line in f:
    try: r=json.loads(line)
    except: continue
    if want_role and norm(r.get("role",""))!=want_role: 
      continue
    hay = " | ".join([
      r.get("conv_title",""),
      r.get("role",""),
      r.get("author_name",""),
      r.get("model",""),
      r.get("dmodel",""),
      r.get("text",""),
    ])
    if term in norm(hay):
      # print compacto (celular)
      out={
        "conv_i": r.get("conv_i"),
        "title":  r.get("conv_title","")[:80],
        "role":   r.get("role"),
        "model":  r.get("model"),
        "len":    r.get("text_len"),
        "text":   (r.get("text","")[:160].replace("\n","\\n")),
      }
      sys.stdout.write(json.dumps(out,ensure_ascii=False)+"\n")
      hits+=1
      if hits>=lim: break

print(f"[D] hits={hits}", file=sys.stderr)
