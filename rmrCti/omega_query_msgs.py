#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json, sys, argparse

def norm(s: str) -> str:
    return (s or "").lower()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", default="omega_msgs.jsonl")
    ap.add_argument("--term", required=True)
    ap.add_argument("--limit", type=int, default=20)
    ap.add_argument("--role", default="", help="filtra role ex: tool|assistant|user|system")
    args = ap.parse_args()

    term = norm(args.term)
    limit = max(1, args.limit)
    want_role = norm(args.role)

    hits = 0
    with open(args.inp, "r", encoding="utf-8") as f:
        for line in f:
            try:
                r = json.loads(line)
            except Exception:
                continue

            if want_role and norm(r.get("role","")) != want_role:
                continue

            hay = " ".join([
                r.get("conv_title",""),
                r.get("role",""),
                r.get("author_name",""),
                r.get("model",""),
                r.get("dmodel",""),
                r.get("text",""),   # 🔥 busca real aqui
            ])

            if term in norm(hay):
                print(json.dumps(r, ensure_ascii=False))
                hits += 1
                if hits >= limit:
                    break

    print(f"[omega_query_msgs] hits={hits}", file=sys.stderr)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
