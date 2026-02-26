#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# omega_metrics.py (stdlib only)
# Entrada: omega_index.jsonl
# Saídas:
# - omega_metrics_summary.json
# - omega_metrics_abc.txt
# - omega_metrics_hotspots.txt
#
# Métricas:
# - Distribuição de kind, role, model
# - Stats line_bytes: mean, std, p50/p90/p99
# - Regressão simples: line_bytes ~ create_time (tendência de tamanho no tempo)
# - "Estoque mínimo" (threshold) = mean + k*std (k=2 por padrão) => hotspots
# - ABC: top títulos/keys por massa (line_bytes)

import json, math, statistics
from collections import Counter, defaultdict

INP="omega_index.jsonl"
OUT_SUM="omega_metrics_summary.json"
OUT_ABC="omega_metrics_abc.txt"
OUT_HOT="omega_metrics_hotspots.txt"

def pct(x, n):
    return 0.0 if n==0 else 100.0*x/n

def quantile(xs, q):
    if not xs: return 0
    xs=sorted(xs)
    k=(len(xs)-1)*q
    f=math.floor(k); c=math.ceil(k)
    if f==c: return xs[int(k)]
    return xs[f]*(c-k) + xs[c]*(k-f)

rows=[]
with open(INP,"r",encoding="utf-8") as f:
    for line in f:
        line=line.strip()
        if not line: continue
        try:
            rows.append(json.loads(line))
        except Exception:
            pass

n=len(rows)
kind=Counter()
role=Counter()
model=Counter()
title=Counter()
sizes=[]
times=[]  # (t, size)

for r in rows:
    k=r.get("kind","other"); kind[k]+=1
    ro=r.get("role",""); 
    if ro: role[ro]+=1
    mo=r.get("model","") or r.get("dmodel","")
    if mo: model[mo]+=1
    t=r.get("title","")
    if t: title[t]+=r.get("line_bytes",0)
    s=r.get("line_bytes",0)
    sizes.append(s)
    ct=r.get("create_time",0.0)
    if ct and s:
        times.append((ct,s))

mean = statistics.fmean(sizes) if sizes else 0.0
std  = statistics.pstdev(sizes) if len(sizes)>1 else 0.0
p50=quantile(sizes,0.50); p90=quantile(sizes,0.90); p99=quantile(sizes,0.99)

# regressão linear simples: y = a + b*t
# b = cov(t,y)/var(t)
reg = {"n":0,"b":0.0,"a":0.0,"r2":0.0}
if len(times) >= 3:
    ts=[t for t,_ in times]
    ys=[y for _,y in times]
    tmean=statistics.fmean(ts)
    ymean=statistics.fmean(ys)
    var_t=statistics.fmean([(t-tmean)**2 for t in ts])
    if var_t > 0:
        cov=statistics.fmean([(ts[i]-tmean)*(ys[i]-ymean) for i in range(len(ts))])
        b=cov/var_t
        a=ymean - b*tmean
        # r2
        ss_tot=statistics.fmean([(y-ymean)**2 for y in ys])
        ss_res=statistics.fmean([(ys[i] - (a + b*ts[i]))**2 for i in range(len(ts))])
        r2=0.0 if ss_tot==0 else max(0.0, 1.0 - (ss_res/ss_tot))
        reg={"n":len(times),"b":b,"a":a,"r2":r2}

# "estoque mínimo": limiar de hotspot
k_sigma=2.0
threshold = mean + k_sigma*std

# hotspots: maiores linhas (acima do threshold)
hot = sorted([(r.get("line_bytes",0), r.get("i",0), r.get("kind",""), r.get("title",""), r.get("model",""), r.get("role",""))
              for r in rows if r.get("line_bytes",0) >= threshold],
             reverse=True)[:120]

# ABC por massa (line_bytes): top títulos
total_mass = sum(title.values()) or 1
ranked = sorted(title.items(), key=lambda kv: kv[1], reverse=True)
abc=[]
cum=0
for t,m in ranked[:300]:
    cum += m
    frac = cum/total_mass
    cls = "A" if frac<=0.80 else ("B" if frac<=0.95 else "C")
    abc.append((cls, m, frac, t))

summary={
    "n": n,
    "kind": {k: {"count": v, "pct": pct(v,n)} for k,v in kind.most_common()},
    "role": {k: {"count": v, "pct": pct(v,n)} for k,v in role.most_common(15)},
    "model": {k: {"count": v, "pct": pct(v,n)} for k,v in model.most_common(20)},
    "sizes": {"mean": mean, "std": std, "p50": p50, "p90": p90, "p99": p99, "threshold_mean_plus_2sigma": threshold},
    "regression_size_vs_create_time": reg,
    "abc_titles_total_mass": total_mass,
    "hotspots_count": len([1 for r in rows if r.get("line_bytes",0) >= threshold]),
}

with open(OUT_SUM,"w",encoding="utf-8") as f:
    json.dump(summary,f,indent=2,ensure_ascii=False)

with open(OUT_ABC,"w",encoding="utf-8") as f:
    f.write("ABC (por massa de bytes de 'title')\n")
    f.write("cls\tbytes\tcum%\ttitle\n")
    for cls,m,frac,t in abc[:80]:
        f.write(f"{cls}\t{m}\t{100.0*frac:.2f}\t{t}\n")

with open(OUT_HOT,"w",encoding="utf-8") as f:
    f.write("HOTSPOTS (line_bytes >= mean + 2*std)\n")
    f.write("bytes\ti\tkind\trole\tmodel\ttitle\n")
    for s,i,k,tit,mo,ro in hot:
        f.write(f"{s}\t{i}\t{k}\t{ro}\t{mo}\t{tit}\n")

print("[omega_metrics] OK")
print(" summary:", OUT_SUM)
print(" abc    :", OUT_ABC)
print(" hot    :", OUT_HOT)
print(f" n={n} mean={mean:.2f} std={std:.2f} threshold={threshold:.2f}")
print(" kinds:", dict(kind))
