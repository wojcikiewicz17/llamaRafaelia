#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
OMEGA — Índice Invertido v1
token -> ocorrências (arquivo, path lógico, ts, contagem)

- streaming (1GB ok)
- stdlib only
- JSONL append-only
"""

import json, sys, argparse, re
from collections import Counter, defaultdict

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{2,}")

def extract_tokens(text, min_len=3, max_len=64):
    c = Counter()
    for t in TOKEN_RE.findall(text):
        tl = t.lower()
        if len(tl) < min_len: continue
        if max_len and len(tl) > max_len: continue
        c[tl] += 1
    return c

def iter_messages(obj, path=""):
    """Gera (path, ts, text) de estruturas comuns do export OpenAI"""
    if isinstance(obj, dict):
        # padrões comuns
        if "message" in obj and isinstance(obj["message"], dict):
            msg = obj["message"]
            text = msg.get("content", "")
            ts = msg.get("create_time") or obj.get("create_time")
            if isinstance(text, str) and text.strip():
                yield (path + "/message", ts, text)

        for k, v in obj.items():
            yield from iter_messages(v, f"{path}/{k}")
    elif isinstance(obj, list):
        for i, v in enumerate(obj):
            yield from iter_messages(v, f"{path}[{i}]")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input_json", help="conversations.json")
    ap.add_argument("--out", default="omega_index.jsonl")
    ap.add_argument("--stats", default="omega_index_stats.json")
    ap.add_argument("--min_token_len", type=int, default=3)
    ap.add_argument("--max_token_len", type=int, default=64)
    ap.add_argument("--top_tokens", type=int, default=200)
    args = ap.parse_args()

    idx = defaultdict(list)
    global_tf = Counter()
    msg_count = 0

    print("loading:", args.input_json)
    with open(args.input_json, "r", encoding="utf-8") as f:
        data = json.load(f)

    for path, ts, text in iter_messages(data):
        msg_count += 1
        toks = extract_tokens(text, args.min_token_len, args.max_token_len)
        for t, c in toks.items():
            idx[t].append({
                "path": path,
                "ts": ts,
                "count": c
            })
            global_tf[t] += c

        if msg_count % 500 == 0:
            print(" msgs:", msg_count)

    # write index
    with open(args.out, "w", encoding="utf-8") as fo:
        for t, occs in idx.items():
            fo.write(json.dumps({
                "token": t,
                "df": len(occs),
                "tf": global_tf[t],
                "occ": occs[:1000]  # cap de segurança
            }, ensure_ascii=False) + "\n")

    stats = {
        "messages": msg_count,
        "unique_tokens": len(idx),
        "top_tokens": global_tf.most_common(args.top_tokens)
    }

    with open(args.stats, "w", encoding="utf-8") as fs:
        json.dump(stats, fs, indent=2, ensure_ascii=False)

    print("OK")
    print(" index:", args.out)
    print(" stats:", args.stats)

if __name__ == "__main__":
    main()
