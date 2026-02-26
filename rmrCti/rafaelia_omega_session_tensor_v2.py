#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
rafaelia_omega_session_tensor_v2.py
Scanner leve de zips/repos para:
- pesar áreas de conhecimento
- contar tipos de arquivos
- extrair tokens dominantes
- gerar resumo por "nicho"

Mobile / Termux friendly
stdlib only
"""

import os, sys, json, zipfile, argparse, re
from collections import Counter, defaultdict

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{2,}")

EXT_MAP = {
    "code": {".c",".h",".py",".sh",".js",".ts",".java",".kt",".cpp",".rs"},
    "docs": {".md",".txt",".rst",".tex"},
    "data": {".json",".csv",".tsv",".yaml",".yml"},
    "bin":  {".so",".a",".o",".bin"},
}

def classify_ext(name):
    ext = os.path.splitext(name.lower())[1]
    for k,v in EXT_MAP.items():
        if ext in v:
            return k
    return "other"

def scan_zip(path):
    stats = Counter()
    tokens = Counter()
    files = 0
    bytes_total = 0

    with zipfile.ZipFile(path, "r") as z:
        for info in z.infolist():
            if info.is_dir():
                continue
            files += 1
            bytes_total += info.file_size
            cls = classify_ext(info.filename)
            stats[cls] += 1

            if info.file_size > 0 and cls in ("code","docs"):
                try:
                    data = z.read(info.filename)
                    text = data.decode("utf-8", errors="ignore")
                    for t in TOKEN_RE.findall(text):
                        tokens[t.lower()] += 1
                except Exception:
                    pass

    return {
        "files": files,
        "bytes": bytes_total,
        "by_class": dict(stats),
        "top_tokens": tokens
    }

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--inputs", nargs="+", required=True)
    ap.add_argument("--out", default="omega_zipscan")
    ap.add_argument("--top_tokens", type=int, default=40)
    args = ap.parse_args()

    summary = {}
    global_tokens = Counter()

    for p in args.inputs:
        name = os.path.basename(p)
        print("scan:", name)
        if not os.path.exists(p):
            print("  ! missing:", p)
            continue
        res = scan_zip(p)
        summary[name] = {
            "files": res["files"],
            "bytes": res["bytes"],
            "by_class": res["by_class"],
            "top_tokens": res["top_tokens"].most_common(args.top_tokens)
        }
        global_tokens.update(res["top_tokens"])

    out_summary = args.out + "_summary.json"
    out_tokens  = args.out + "_tokens.json"

    with open(out_summary, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2, ensure_ascii=False)

    with open(out_tokens, "w", encoding="utf-8") as f:
        json.dump(global_tokens.most_common(200), f, indent=2, ensure_ascii=False)

    print("OK")
    print(" summary:", out_summary)
    print(" tokens :", out_tokens)

if __name__ == "__main__":
    main()
