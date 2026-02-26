#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, json, zipfile, argparse, re, math, fnmatch, csv
from collections import Counter, defaultdict
from dataclasses import dataclass

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{2,}")

EXT_MAP = {
  "code": {".c",".h",".py",".sh",".js",".ts",".java",".kt",".cpp",".rs",".go",".lua",".rb",".php"},
  "docs": {".md",".txt",".rst",".tex",".html",".htm",".json"},  # <- aqui: json/html viram "docs"
  "data": {".csv",".tsv",".yaml",".yml",".toml",".ini"},
  "bin":  {".so",".a",".o",".bin",".dex",".class"},
}

ANCHOR_TOKENS = {"ethica","bitraf","zipraf","rafaelia","trinity","toroid","spiral","omega","gaia","rmr","aether","pre6","hash","blake3"}

def classify_ext(name: str) -> str:
  ext = os.path.splitext(name.lower())[1]
  for k,v in EXT_MAP.items():
    if ext in v: return k
  return "other"

def allow_path(path: str, includes, excludes) -> bool:
  p = path.replace("\\","/")
  for pat in excludes or []:
    if fnmatch.fnmatch(p, pat): return False
  if includes:
    return any(fnmatch.fnmatch(p, pat) for pat in includes)
  return True

def is_probably_binary(raw: bytes) -> bool:
  if not raw: return False
  return (raw.count(b"\x00") / max(1, len(raw))) > 0.02

def read_text(raw: bytes, max_chars: int) -> str:
  t = raw.decode("utf-8", errors="ignore")
  return t[:max_chars] if max_chars > 0 else t

def shannon_entropy(text: str) -> float:
  if not text: return 0.0
  freq = Counter(text); n = len(text)
  ent = 0.0
  for _,k in freq.items():
    p = k/n; ent -= p*math.log2(p)
  return ent

def extract_tokens(text: str, min_len: int, max_len: int) -> Counter:
  c = Counter()
  for t in TOKEN_RE.findall(text):
    t=t.lower()
    if len(t) < min_len: continue
    if max_len > 0 and len(t) > max_len: continue
    c[t]+=1
  return c

@dataclass
class FileSignal:
  path: str
  cls: str
  size: int
  entropy: float
  tokens: Counter

def pack_results(sigs, stats, lang, files_total, bytes_total, args):
  tf, df = Counter(), Counter()
  ent_by = defaultdict(list)
  cooc = Counter()

  for s in sigs:
    ent_by[s.cls].append(s.entropy)
    tf.update(s.tokens)
    for t in s.tokens: df[t]+=1
    topk = [t for t,_ in s.tokens.most_common(args.cooc_topk)]
    for i in range(len(topk)):
      for j in range(i+1,len(topk)):
        a,b = sorted((topk[i], topk[j]))
        cooc[(a,b)] += 1

  n = max(1, len(sigs))
  tfidf = {t: tf[t]*(math.log((n+1)/(df[t]+1))+1) for t in tf}
  emerg = {t:(df[t]/n)*(1.25 if any(a in t for a in ANCHOR_TOKENS) else 1.0) for t in df}
  latent = {t: tfidf[t]*(1-df[t]/n) for t in tfidf}

  needs = {}
  ratio = stats.get("code",0)/max(1,stats.get("docs",0))
  needs["code_docs_ratio"]=ratio
  needs["docs_missing_hint"]=ratio > args.need_ratio_threshold
  test_cnt = sum(tf.get(x,0) for x in ("test","tests","pytest","unittest"))
  needs["tests_hint_count"]=test_cnt
  needs["tests_missing_hint"]=test_cnt==0

  ent_stats = {k:{"mean":sum(v)/len(v),"min":min(v),"max":max(v),"n":len(v)} for k,v in ent_by.items() if v}

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
    "needs": needs
  }

def scan_zip(path: str, args) -> dict:
  stats, lang = Counter(), Counter()
  sigs=[]; bytes_total=0; files_total=0

  with zipfile.ZipFile(path,"r") as z:
    for info in z.infolist():
      if info.is_dir(): continue
      rel = info.filename
      if not allow_path(rel, args.include, args.exclude): continue

      files_total += 1
      bytes_total += info.file_size
      cls = classify_ext(rel)
      stats[cls] += 1
      lang[os.path.splitext(rel.lower())[1] or ""] += 1

      if info.file_size <= 0: continue
      if cls not in ("code","docs"): continue

      # leitura parcial (amostra), sem explodir RAM
      try:
        with z.open(info, "r") as f:
          raw = f.read(args.zip_sample_bytes)
        if is_probably_binary(raw): continue
        txt = read_text(raw, args.max_chars)
        ent = shannon_entropy(txt)
        toks = extract_tokens(txt, args.min_token_len, args.max_token_len)
        sigs.append(FileSignal(rel, cls, info.file_size, ent, toks))
      except Exception:
        continue

  return pack_results(sigs, stats, lang, files_total, bytes_total, args)

def write_md(path, allsum, args):
  out=["# RAFAELIA Ω — Export Sampler\n"]
  for name,r in allsum.items():
    out += [f"## {name}",
            f"- files_total: {r['files_total']}",
            f"- bytes_total: {r['bytes_total']}",
            f"- classes: {r['by_class']}",
            f"- docs_scanned: {r['n_docs_scanned']}",
            "",
            "### Top Emergent",
            *[f"- `{t}`: {s:.3f}" for t,s in r["top_emergent"][:20]],
            "",
            "### Top Latent",
            *[f"- `{t}`: {s:.3f}" for t,s in r["top_latent"][:20]],
            ""]
  open(path,"w",encoding="utf-8").write("\n".join(out))

def main():
  ap=argparse.ArgumentParser()
  ap.add_argument("--inputs", nargs="+", required=True)
  ap.add_argument("--out", default="omega_export_v3b")
  ap.add_argument("--include", nargs="*", default=["*.json","*.html","*.htm"])
  ap.add_argument("--exclude", nargs="*", default=[])
  ap.add_argument("--top_tokens", type=int, default=80)
  ap.add_argument("--top_cooc", type=int, default=60)
  ap.add_argument("--cooc_topk", type=int, default=10)
  ap.add_argument("--min_token_len", type=int, default=3)
  ap.add_argument("--max_token_len", type=int, default=64)
  ap.add_argument("--max_chars", type=int, default=120000)
  ap.add_argument("--zip_sample_bytes", type=int, default=2*1024*1024) # 2MB por arquivo
  ap.add_argument("--need_ratio_threshold", type=float, default=4.0)
  args=ap.parse_args()

  allsum={}
  for p in args.inputs:
    name=os.path.basename(p.rstrip("/"))
    print("scan:", name)
    allsum[name]=scan_zip(p,args)

  js=args.out+"_summary.json"
  md=args.out+"_report.md"
  with open(js,"w",encoding="utf-8") as f:
    json.dump(allsum,f,indent=2,ensure_ascii=False)
  write_md(md, allsum, args)
  print("OK"); print(" summary:", js); print(" report :", md)

if __name__=="__main__":
  main()
