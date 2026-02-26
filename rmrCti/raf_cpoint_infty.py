#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RAFAELIA / CPOINT — PRT + I∞
- Roda TRIAD_ENGINE do cpoint.zip
- Extrai salts do triad_trace.csv
- Gera bytes via Permutivas Randômicas Totais (ROTL + XOR + mult mod 256)
- Mede I∞ (infinito realizável): entropia + espalhamento + anti-curto-ciclo
"""

from __future__ import annotations
import argparse, csv, json, math, shutil, tempfile, zipfile
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Dict, List, Tuple, Optional


def ensure_dir(p: Path) -> None:
    p.mkdir(parents=True, exist_ok=True)

def rotl8(x: int, r: int) -> int:
    r &= 7
    x &= 0xFF
    return ((x << r) | (x >> (8 - r))) & 0xFF

def shannon_entropy_u8(seq: List[int]) -> float:
    if not seq:
        return 0.0
    freq = [0] * 256
    for b in seq:
        freq[b & 0xFF] += 1
    n = len(seq)
    H = 0.0
    for c in freq:
        if c:
            p = c / n
            H -= p * math.log2(p)
    return H

def markov_collision(seq: List[int]) -> float:
    if len(seq) < 2:
        return 0.0
    trans: Dict[Tuple[int, int], int] = {}
    prev = seq[0] & 0xFF
    for b in seq[1:]:
        cur = b & 0xFF
        trans[(prev, cur)] = trans.get((prev, cur), 0) + 1
        prev = cur
    T = len(seq) - 1
    s = 0.0
    for cnt in trans.values():
        p = cnt / T
        s += p * p
    return s

def approx_cycle_ratio(seq: List[int]) -> float:
    if not seq:
        return 0.0
    distinct = len(set([b & 0xFF for b in seq]))
    return min(1.0, distinct / 256.0)

def i_infty_metric(seq: List[int]) -> Dict[str, float]:
    H = shannon_entropy_u8(seq)
    M = markov_collision(seq)
    R = approx_cycle_ratio(seq)
    if M <= 0:
        I = 0.0
    else:
        I = (H / 8.0) * (1.0 / (256.0 * M)) * R
    return {"H_bits": H, "M_markov_collision": M, "R_distinct_ratio": R, "I_infty": I}

def theta_k(k: int, N: int) -> float:
    return (2.0 * math.pi * (k % N)) / float(N)

def Q6(theta: float) -> int:
    return int(round((6.0 * (theta % (2.0 * math.pi))) / (2.0 * math.pi))) % 6

def quad2(theta: float) -> int:
    c = math.cos(theta)
    s = math.sin(theta)
    if c >= 0 and s >= 0:
        return 0
    if c < 0 and s >= 0:
        return 1
    if c < 0 and s < 0:
        return 2
    return 3

@dataclass
class PRTParams:
    Nphase: int = 70
    layers: int = 56
    hyperforms: int = 42
    a: int = 5
    b: int = 6
    c: int = 11
    alpha: int = 205   # must be odd
    salt_gain: int = 17

def prt_step(byte_in: int, k: int, L: int, H: int, salt: int, p: PRTParams) -> int:
    th = theta_k(k, p.Nphase)
    r = (Q6(th) & 7)
    sigma = (p.a * k + p.b * L + p.c * H + p.salt_gain * (salt & 0xFF)) & 0xFF
    x = ((byte_in ^ sigma) * (p.alpha & 0xFF)) & 0xFF
    return rotl8(x, r)

def prt_generate(salts: List[int], p: PRTParams, seed_byte: int = 0x42) -> List[int]:
    out: List[int] = []
    b = seed_byte & 0xFF
    for t, s in enumerate(salts):
        k = t % p.Nphase
        L = t % p.layers
        H = t % p.hyperforms
        b = prt_step(b, k, L, H, s & 0xFF, p)
        out.append(b)
    return out

@dataclass
class TriadRunCfg:
    steps: int = 2000
    seed: int = 42
    R: float = 2.0
    r: float = 0.7
    alpha: float = math.pi / 7.0
    beta: float = math.pi / 9.0
    kappa: float = 0.22
    z0_re: float = 0.0
    z0_im: float = 0.0
    lam: float = 1000.0
    z_cap: float = 1e6

def run_triad_from_zip(zip_path: Path, out_dir: Path, cfg: TriadRunCfg) -> Tuple[Path, Path]:
    ensure_dir(out_dir)
    triad_out = out_dir / "triad"
    ensure_dir(triad_out)

    tmp = tempfile.mkdtemp(prefix="cpoint_")
    try:
        with zipfile.ZipFile(zip_path, "r") as zf:
            zf.extractall(tmp)

        triad_py = Path(tmp) / "src" / "triad_engine.py"
        if not triad_py.exists():
            raise FileNotFoundError(f"triad_engine.py not found inside {zip_path}")

        import importlib.util
        spec = importlib.util.spec_from_file_location("triad_engine", str(triad_py))
        if spec is None or spec.loader is None:
            raise RuntimeError("Failed to import triad_engine")
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)  # type: ignore

        Params = getattr(mod, "Params")
        run_fn = getattr(mod, "run")

        params = Params(
            steps=cfg.steps, seed=cfg.seed, R=cfg.R, r=cfg.r,
            alpha=cfg.alpha, beta=cfg.beta, kappa=cfg.kappa,
            z0_re=cfg.z0_re, z0_im=cfg.z0_im, lam=cfg.lam, z_cap=cfg.z_cap
        )
        run_fn(params, str(triad_out))

        trace_csv = triad_out / "triad_trace.csv"
        report_json = triad_out / "triad_report.json"
        if not trace_csv.exists() or not report_json.exists():
            raise RuntimeError("triad_engine did not produce expected outputs.")
        return trace_csv, report_json
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

def parse_triad_trace_for_salts(trace_csv: Path, max_rows: Optional[int] = None) -> List[int]:
    salts: List[int] = []
    with trace_csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for idx, row in enumerate(reader):
            if max_rows is not None and idx >= max_rows:
                break

            picked = None

            # prefer dr-like
            for key in ("dr_delta", "dr", "pi_n", "dr9"):
                if key in row and row[key].strip():
                    try:
                        picked = int(float(row[key]))
                        break
                    except Exception:
                        pass

            # delta magnitude
            if picked is None:
                for key in ("delta_abs", "D_abs", "abs_D", "abs_delta"):
                    if key in row and row[key].strip():
                        try:
                            picked = int(float(row[key]) * 1000.0)
                            break
                        except Exception:
                            pass

            # |z|
            if picked is None:
                for key in ("abs_z", "z_abs", "|z|"):
                    if key in row and row[key].strip():
                        try:
                            picked = int(float(row[key]) * 1_000_000.0)
                            break
                        except Exception:
                            pass

            # fallback hash
            if picked is None:
                s = "|".join([f"{k}={row[k]}" for k in sorted(row.keys())])
                h = 0
                for ch in s.encode("utf-8", errors="ignore"):
                    h = (h * 131 + ch) & 0xFF
                picked = h

            salts.append(picked & 0xFF)

    return salts

def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip", type=str, default="cpoint.zip")
    ap.add_argument("--out", type=str, default="out_run")
    ap.add_argument("--steps", type=int, default=2000)
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--alpha", type=float, default=math.pi / 7.0)
    ap.add_argument("--beta", type=float, default=math.pi / 9.0)
    ap.add_argument("--kappa", type=float, default=0.22)
    ap.add_argument("--R", type=float, default=2.0)
    ap.add_argument("--r", type=float, default=0.7)
    ap.add_argument("--lam", type=float, default=1000.0)

    ap.add_argument("--Nphase", type=int, default=70)
    ap.add_argument("--layers", type=int, default=56)
    ap.add_argument("--hyperforms", type=int, default=42)
    ap.add_argument("--prt_alpha", type=int, default=205)
    ap.add_argument("--prt_a", type=int, default=5)
    ap.add_argument("--prt_b", type=int, default=6)
    ap.add_argument("--prt_c", type=int, default=11)
    ap.add_argument("--seed_byte", type=lambda x: int(x, 0), default="0x42")
    args = ap.parse_args()

    zip_path = Path(args.zip).resolve()
    out_dir = Path(args.out).resolve()
    ensure_dir(out_dir)

    triad_cfg = TriadRunCfg(
        steps=args.steps, seed=args.seed, R=args.R, r=args.r,
        alpha=args.alpha, beta=args.beta, kappa=args.kappa, lam=args.lam
    )

    trace_csv, report_json = run_triad_from_zip(zip_path, out_dir, triad_cfg)
    salts = parse_triad_trace_for_salts(trace_csv)

    prt_params = PRTParams(
        Nphase=args.Nphase, layers=args.layers, hyperforms=args.hyperforms,
        a=args.prt_a, b=args.prt_b, c=args.prt_c,
        alpha=(args.prt_alpha | 1)
    )
    bytes_out = prt_generate(salts, prt_params, seed_byte=args.seed_byte)

    metrics = i_infty_metric(bytes_out)

    prt_out = out_dir / "prt"
    ensure_dir(prt_out)

    bytes_csv = prt_out / "prt_bytes.csv"
    with bytes_csv.open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["t","k70","L56","H42","theta","q6","quad","salt_u8","byte_u8"])
        for t, (s, b) in enumerate(zip(salts, bytes_out)):
            k = t % prt_params.Nphase
            L = t % prt_params.layers
            H = t % prt_params.hyperforms
            th = theta_k(k, prt_params.Nphase)
            w.writerow([t, k, L, H, th, Q6(th), quad2(th), s & 0xFF, b & 0xFF])

    prt_report = {
        "triad_cfg": asdict(triad_cfg),
        "prt_params": asdict(prt_params),
        "paths": {
            "triad_trace_csv": str(trace_csv),
            "triad_report_json": str(report_json),
            "prt_bytes_csv": str(bytes_csv),
        },
        "metrics": metrics,
        "notes": {
            "I_infty_meaning": "Higher ~ more 'infinite-like' within finite byte space (uniformity + spread + avoid short cycles).",
            "H_bits_max": 8,
            "markov_collision_ideal": 1/256,
        }
    }
    (prt_out / "prt_report.json").write_text(json.dumps(prt_report, ensure_ascii=False, indent=2), encoding="utf-8")

    print("=== RAFAELIA / CPOINT — PRT + I∞ ===")
    print(f"TRIAD trace: {trace_csv}")
    print(f"TRIAD report: {report_json}")
    print(f"PRT bytes: {bytes_csv}")
    print(f"PRT report: {prt_out / 'prt_report.json'}")
    print("")
    print(f"H (bits): {metrics['H_bits']:.4f}  (max 8)")
    print(f"M (Markov collision): {metrics['M_markov_collision']:.6f}  (ideal ~ {1/256:.6f})")
    print(f"R (distinct ratio): {metrics['R_distinct_ratio']:.4f}")
    print(f"I∞: {metrics['I_infty']:.6f}")

if __name__ == "__main__":
    main()
