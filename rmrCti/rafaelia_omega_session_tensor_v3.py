#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
rafaelia_omega_session_tensor_v3.py
Scanner leve (ZIP + diretórios) para:
- pesar áreas (code/docs/data/bin/other)
- contar tipos e linguagens por extensão
- extrair tokens dominantes
- medir "latentes" (raros e informativos) via TF-IDF simples
- medir "emergentes" (transversais) via cobertura (docfreq) e coocorrência
- gerar nichos (clusters leves por assinatura de tokens)
- detectar necessidades (assimetria code/docs, falta de testes, etc.)

Mobile / Termux friendly
stdlib only
"""

import os, sys, json, zipfile, argparse, re, math, fnmatch, csv
from collections import Counter, defaultdict
from dataclasses import dataclass

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{2,}")

EXT_MAP = {
    "code": {".c",".h",".py",".sh",".js",".ts",".java",".kt",".cpp",".rs",".go",".lua",".rb",".php"},
    "docs": {".md",".txt",".rst",".tex"},
    "data": {".json",".csv",".tsv",".yaml",".yml",".toml",".ini"},
    "bin":  {".so",".a",".o",".bin",".dex",".class"},
}

ANCHOR_TOKENS = {
    "ethica","bitraf","zipraf","rafaelia","trinity","toroid","spiral",
    "omega","gaia","rmr","aether","pre6","hash","blake3"
}

def classify_ext(name: str) -> str:
    ext = os.path.splitext(name.lower())[1]
    for k, v in EXT_MAP.items():
        if ext in v:
            return k
    return "other"

def is_probably_binary(raw: bytes) -> bool:
    if not raw:
        return False
    return (raw.count(b"\x00") / max(1, len(raw))) > 0.02

def shannon_entropy(text: str) -> float:
    if not text:
        return 0.0
    freq = Counter(text)
    n = len(text)
    ent = 0.0
    for c, k in freq.items():
        p = k / n
        ent -= p * math.log2(p)
    return ent

def allow_path(path: str, includes, excludes) -> bool:
    p = path.replace("\\", "/")
    for pat in excludes or []:
        if fnmatch.fnmatch(p, pat):
            return False
    if includes:
        return any(fnmatch.fnmatch(p, pat) for pat in includes)
    return True

def iter_dir_files(root: str, max_depth: int):
    root = os.path.abspath(root)
    base = root.rstrip(os.sep).count(os.sep)
    for d, ds, fs in os.walk(root):
        depth = d.rstrip(os.sep).count(os.sep) - base
        if max_depth >= 0 and depth > max_depth:
            ds[:] = []
            continue
        for f in fs:
            yield os.path.join(d, f)

def read_text(raw: bytes, max_chars: int) -> str:
    t = raw.decode("utf-8", errors="ignore")
    return t[:max_chars] if max_chars > 0 else t

@dataclass
class FileSignal:
    path: str
    cls: str
    size: int
    entropy: float
    tokens: Counter

def extract_tokens(text: str, min_len: int, max_len: int) -> Counter:
    c = Counter()
    for t in TOKEN_RE.findall(text):
        t = t.lower()
        if len(t) < min_len:
            continue
        if max_len > 0 and len(t) > max_len:
            continue
        c[t] += 1
    return c

def scan_zip(path: str, args) -> dict:
    stats, lang = Counter(), Counter()
    sigs, bytes_total, files_total = [], 0, 0
    with zipfile.ZipFile(path, "r") as z:
        for i in z.infolist():
            if i.is_dir():
                continue
            rel = i.filename
            if not allow_path(rel, args.include, args.exclude):
                continue
            files_total += 1
            bytes_total += i.file_size
            cls = classify_ext(rel)
            stats[cls] += 1
            lang[os.path.splitext(rel.lower())[1] or ""] += 1
            if i.file_size <= 0 or i.file_size > args.max_file_mb * 1024 * 1024:
                continue
            if cls not in ("code","docs"):
                continue
            raw = z.read(rel)
            if is_probably_binary(raw):
                continue
            txt = read_text(raw, args.max_chars)
            ent = shannon_entropy(txt)
            toks = extract_tokens(txt, args.min_token_len, args.max_token_len)
            sigs.append(FileSignal(rel, cls, i.file_size, ent, toks))
    return pack_results(sigs, stats, lang, files_total, bytes_total, args)

def scan_dir(path: str, args) -> dict:
    stats, lang = Counter(), Counter()
    sigs, bytes_total, files_total = [], 0, 0
    for fp in iter_dir_files(path, args.max_depth):
        rel = os.path.relpath(fp, path).replace("\\","/")
        if not allow_path(rel, args.include, args.exclude):
            continue
        try:
            st = os.stat(fp)
        except Exception:
            continue
        files_total += 1
        bytes_total += st.st_size
        cls = classify_ext(rel)
        stats[cls] += 1
        lang[os.path.splitext(rel.lower())[1] or ""] += 1
        if st.st_size <= 0 or st.st_size > args.max_file_mb * 1024 * 1024:
            continue
        if cls not in ("code","docs"):
            continue
        with open(fp,"rb") as f:
            raw = f.read(args.max_bytes)
        if is_probably_binary(raw):
            continue
        txt = read_text(raw, args.max_chars)
        ent = shannon_entropy(txt)
        toks = extract_tokens(txt, args.min_token_len, args.max_token_len)
        sigs.append(FileSignal(rel, cls, st.st_size, ent, toks))
    return pack_results(sigs, stats, lang, files_total, bytes_total, args)

def pack_results(sigs, stats, lang, files_total, bytes_total, args):
    tf, df, ent_by_cls = Counter(), Counter(), defaultdict(list)
    cooc = Counter()
    for s in sigs:
        ent_by_cls[s.cls].append(s.entropy)
        tf.update(s.tokens)
        for t in s.tokens:
            df[t] += 1
        topk = [t for t,_ in s.tokens.most_common(args.cooc_topk)]
        for i in range(len(topk)):
            for j in range(i+1,len(topk)):
                a,b = sorted((topk[i],topk[j]))
                cooc[(a,b)] += 1

    n = max(1,len(sigs))
    tfidf = {t: tf[t]*(math.log((n+1)/(df[t]+1))+1) for t in tf}
    emerg = {t:(df[t]/n)*(1.25 if any(a in t for a in ANCHOR_TOKENS) else 1.0) for t in df}
    latent = {t: tfidf[t]*(1-df[t]/n) for t in tfidf}

    niches = defaultdict(list)
    for s in sigs:
        sig = tuple(t for t,_ in s.tokens.most_common(args.niche_topn))
        niches[sig].append(s.path)

    needs = {}
    ratio = stats.get("code",0)/max(1,stats.get("docs",0))
    needs["code_docs_ratio"] = ratio
    needs["docs_missing_hint"] = ratio > args.need_ratio_threshold
    test_cnt = sum(tf.get(x,0) for x in ("test","tests","pytest","unittest"))
    needs["tests_hint_count"] = test_cnt
    needs["tests_missing_hint"] = test_cnt==0

    ent_stats = {k:{
        "mean": sum(v)/len(v),
        "min": min(v),
        "max": max(v),
        "n": len(v)
    } for k,v in ent_by_cls.items() if v}

    return {
        "files_total": files_total,
        "bytes_total": bytes_total,
        "by_class": dict(stats),
        "by_ext": dict(lang.most_common(40)),
        "n_docs_scanned": n,
        "entropy": ent_stats,
        "top_tokens_tf": tf.most_common(args.top_tokens),
        "top_latent": sorted(latent.items(), key=lambda x:x[1], reverse=True)[:args.top_tokens],
        "top_emergent": sorted(emerg.items(), key=lambda x:x[1], reverse=True)[:args.top_tokens],
        "cooc_top": [{"a":a,"b":b,"w":w} for (a,b),w in cooc.most_common(args.top_cooc)],
        "niches": [{"signature":list(sig),"files":paths[:args.niche_max_files],"n":len(paths)}
                   for sig,paths in sorted(niches.items(), key=lambda kv:len(kv[1]), reverse=True)[:args.niche_max]],
        "needs": needs
    }

def write_md(path, allsum, args):
    out=[]
    out.append("# RAFAELIA Ω — Session Tensor v3\n")
    for name,r in allsum.items():
        out.append(f"## {name}")
        out.append(f"- files: {r['files_total']}")
        out.append(f"- bytes: {r['bytes_total']}")
        out.append(f"- classes: {r['by_class']}")
        out.append(f"- docs_scanned: {r['n_docs_scanned']}\n")
        out.append("### Needs")
        for k,v in r["needs"].items():
            out.append(f"- {k}: {v}")
        out.append("\n### Top Latent")
        out.extend([f"- `{t}`: {s:.3f}" for t,s in r["top_latent"][:12]])
        out.append("\n### Top Emergent")
        out.extend([f"- `{t}`: {s:.3f}" for t,s in r["top_emergent"][:12]])
        out.append("")
    with open(path,"w",encoding="utf-8") as f:
        f.write("\n".join(out))

def write_csv(path, allsum):
    cols=["name","files_total","bytes_total","code","docs","data","bin","other","code_docs_ratio","tests_hint_count"]
    with open(path,"w",encoding="utf-8",newline="") as f:
        w=csv.DictWriter(f,fieldnames=cols); w.writeheader()
        for name,r in allsum.items():
            w.writerow({
                "name":name,
                "files_total":r["files_total"],
                "bytes_total":r["bytes_total"],
                "code":r["by_class"].get("code",0),
                "docs":r["by_class"].get("docs",0),
                "data":r["by_class"].get("data",0),
                "bin":r["by_class"].get("bin",0),
                "other":r["by_class"].get("other",0),
                "code_docs_ratio":r["needs"].get("code_docs_ratio",0),
                "tests_hint_count":r["needs"].get("tests_hint_count",0)
            })

def is_zip(p): return p.lower().endswith(".zip") and os.path.isfile(p)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--inputs",nargs="+",required=True)
    ap.add_argument("--out",default="omega_scan_v3")
    ap.add_argument("--top_tokens",type=int,default=40)
    ap.add_argument("--top_cooc",type=int,default=60)
    ap.add_argument("--cooc_topk",type=int,default=10)
    ap.add_argument("--max_file_mb",type=int,default=4)
    ap.add_argument("--max_bytes",type=int,default=2*1024*1024)
    ap.add_argument("--max_chars",type=int,default=300_000)
    ap.add_argument("--min_token_len",type=int,default=3)
    ap.add_argument("--max_token_len",type=int,default=64)
    ap.add_argument("--max_depth",type=int,default=12)
    ap.add_argument("--include",nargs="*",default=[])
    ap.add_argument("--exclude",nargs="*",default=["*/.git/*","*/node_modules/*","*/build/*","*/dist/*"])
    ap.add_argument("--niche_topn",type=int,default=8)
    ap.add_argument("--niche_max",type=int,default=40)
    ap.add_argument("--niche_max_files",type=int,default=30)
    ap.add_argument("--need_ratio_threshold",type=float,default=4.0)
    args=ap.parse_args()

    allsum={}
    for p in args.inputs:
        name=os.path.basename(p.rstrip("/"))
        print("scan:",name)
        if not os.path.exists(p):
            print("  ! missing:",p); continue
        allsum[name]=scan_zip(p,args) if is_zip(p) else scan_dir(p,args)

    js=args.out+"_summary.json"
    md=args.out+"_report.md"
    cv=args.out+"_table.csv"

    with open(js,"w",encoding="utf-8") as f:
        json.dump(allsum,f,indent=2,ensure_ascii=False)
    write_md(md,allsum,args)
    write_csv(cv,allsum)

    print("OK")
    print(" summary:",js)
    print(" report :",md)
    print(" table  :",cv)

if __name__=="__main__":
    main()
