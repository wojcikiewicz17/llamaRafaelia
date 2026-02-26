#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
triad_cti_couple.py
CTI -> TRIAD + TEXTURE10 (10×10 bits) + paridades + ética (love_guard)

Aceita dialetos CTI:
  H | entropy
  F | flips
  X_bad | x_bad

TEXTURE10:
  - 8×8 data bits (hash determinístico de campos CTI/TRIAD)
  - paridade por linha/coluna (8 bits) + paridade global
  - linha 9 e coluna 9 (borda) carregam meta-bits (gate/J_n/flags)
  - exporta:
      grid10x10         (string 10 linhas)
      grid10x10_hex     (13 bytes hex = 104 bits, 4 bits padding)
      par_row8, par_col8, par_global
"""

import json, math, argparse, os, hashlib
from collections import Counter

# ---------------- util ----------------

def dr9(x: int) -> int:
    x = abs(int(x))
    if x == 0:
        return 9
    r = x % 9
    return 9 if r == 0 else r

def complex_sqrt(z: complex) -> complex:
    r = abs(z)
    if r == 0:
        return 0j
    a, b = z.real, z.imag
    t = math.sqrt((r + a) / 2.0)
    u = math.sqrt((r - a) / 2.0)
    if b < 0:
        u = -u
    return complex(t, u)

def bhaskara_delta(c: complex) -> complex:
    return 1.0 - 4.0 * c

def fixpoints_from_c(c: complex):
    D = bhaskara_delta(c)
    s = complex_sqrt(D)
    return (1.0 + s) / 2.0, (1.0 - s) / 2.0, D

def stable_fixpoint(z: complex) -> bool:
    return abs(2.0 * z) < 1.0

# Fibonacci-Rafael
def fr_seq(n: int):
    if n <= 0:
        return [0]
    if n == 1:
        return [0, 2]
    f = [0, 2, 4]
    for _ in range(3, n + 1):
        f.append(f[-1] + f[-2] - 1)
    return f

def fr_rev_step(Fn: int, Fnm1: int):
    return Fn - Fnm1 + 1

# ---------------- mapping CTI -> toro ----------------

def map_uv(rec, alpha, beta):
    n = int(rec.get("idx", 0))

    # aceita H | entropy
    H = float(rec.get("H", rec.get("entropy", 0.0)))
    ones = int(rec.get("ones", 0))

    # aceita F | flips
    flips = int(rec.get("F", rec.get("flips", 0)))

    du = (H / 8.0) * (2.0 * math.pi)
    dv = (dr9(ones) / 9.0) * (2.0 * math.pi)
    jit = (dr9(flips) / 9.0) * (math.pi / 32.0)

    u = n * alpha + du + jit
    v = n * beta  + dv - jit
    return u, v

def torus_c(u, v, R, r, kappa):
    s = (R + r * math.cos(v))
    x = s * math.cos(u)
    y = s * math.sin(u)
    return kappa * complex(x, y)

# ---------------- TEXTURE10 (10×10 bits) ----------------

def _bits_from_hash(seed: str, nbits: int) -> list:
    # sha256 => 256 bits; determinístico, stdlib only
    d = hashlib.sha256(seed.encode("utf-8")).digest()
    out = []
    for byte in d:
        for k in range(7, -1, -1):
            out.append((byte >> k) & 1)
            if len(out) >= nbits:
                return out
    # fallback (não deve acontecer)
    while len(out) < nbits:
        out.append(0)
    return out

def _xor_parity(bits: list) -> int:
    p = 0
    for b in bits:
        p ^= (b & 1)
    return p

def _pack_bits_hex(bits100: list) -> str:
    # 100 bits -> pad para 104 bits (13 bytes) => 4 bits de padding no final
    bits = list(bits100)
    while len(bits) < 104:
        bits.append(0)
    # pack big-endian bits
    ba = bytearray()
    for i in range(0, 104, 8):
        v = 0
        for j in range(8):
            v = (v << 1) | (bits[i + j] & 1)
        ba.append(v)
    return ba.hex()

def _grid_to_ascii(grid10: list) -> str:
    # grid10: list of 10 lists of 10 bits
    return "\n".join("".join("1" if b else "0" for b in row) for row in grid10)

def texture10_build(rec: dict, gate: int, J_n: int, stable_any: int, love_guard: int) -> dict:
    # seed determinístico (CTI + TRIAD)
    # (usa só campos estáveis, não depende de ordem)
    H = float(rec.get("H", rec.get("entropy", 0.0)))
    E = int(rec.get("E", 0))
    F = int(rec.get("F", rec.get("flips", 0)))
    ones = int(rec.get("ones", 0))
    idx = int(rec.get("idx", 0))
    off = int(rec.get("off", 0))
    fid = str(rec.get("fid_crc32c", rec.get("fid_pad", rec.get("fid_raw", ""))))

    seed = f"fid={fid}|idx={idx}|off={off}|E={E}|F={F}|ones={ones}|H={H:.6f}|gate={gate}|J={J_n}|st={stable_any}|lg={love_guard}|X={int(rec.get('X_bad', rec.get('x_bad', 0)))}"
    data64 = _bits_from_hash(seed, 64)  # 8×8 = 64 bits

    # grid 10×10 inicial
    grid = [[0]*10 for _ in range(10)]

    # preencher 8×8 (0..7,0..7)
    k = 0
    for r in range(8):
        for c in range(8):
            grid[r][c] = data64[k]
            k += 1

    # paridade por linha (coluna 8) e por coluna (linha 8)
    par_row8 = 0
    for r in range(8):
        pr = _xor_parity(grid[r][0:8])
        grid[r][8] = pr
        par_row8 |= (pr & 1) << (7 - r)

    par_col8 = 0
    for c in range(8):
        pc = _xor_parity([grid[r][c] for r in range(8)])
        grid[8][c] = pc
        par_col8 |= (pc & 1) << (7 - c)

    # paridade global (8,8) = xor de todas as 64 + paridades (estilo ECC simples)
    all_bits = []
    for r in range(8):
        all_bits.extend(grid[r][0:8])
    all_bits.extend([grid[r][8] for r in range(8)])
    all_bits.extend([grid[8][c] for c in range(8)])
    par_global = _xor_parity(all_bits)
    grid[8][8] = par_global

    # Meta-border: última linha (r=9) e última coluna (c=9)
    # Vamos codificar 10 meta-bits na coluna 9 (r 0..9) e 10 meta-bits na linha 9 (c 0..9)
    # meta bits: gate (4), J_n (2), stable_any (1), X_bad (1), love_guard (1), dr9(ones) LSB (1)
    X_bad = int(rec.get("X_bad", rec.get("x_bad", 0))) & 1
    meta = []
    meta += [(gate >> 3) & 1, (gate >> 2) & 1, (gate >> 1) & 1, gate & 1]          # 4
    meta += [(J_n >> 1) & 1, J_n & 1]                                                # 2
    meta += [stable_any & 1]                                                         # 1
    meta += [X_bad]                                                                  # 1
    meta += [love_guard & 1]                                                         # 1
    meta += [dr9(ones) & 1]                                                          # 1
    # total = 10 bits
    meta = meta[:10]

    # coluna 9: r 0..9
    for r in range(10):
        grid[r][9] = meta[r]

    # linha 9: repetir meta mas com inversão (para ter assimetria) usando XOR 1
    for c in range(10):
        grid[9][c] = meta[c] ^ 1

    # Agora exportar 100 bits (10×10) em ordem row-major
    bits100 = []
    for r in range(10):
        for c in range(10):
            bits100.append(grid[r][c])

    return {
        "grid10x10": _grid_to_ascii(grid),
        "grid10x10_hex": _pack_bits_hex(bits100),
        "par_row8": f"{par_row8:02x}",   # 8 bits (hex)
        "par_col8": f"{par_col8:02x}",   # 8 bits (hex)
        "par_global": int(par_global),
        "meta_seed_crc32": f"{(zlib_crc32(seed.encode('utf-8')) & 0xFFFFFFFF):08x}",
    }

def zlib_crc32(data: bytes) -> int:
    # stdlib zlib.crc32 sem importar zlib no topo (micro)
    import zlib
    return zlib.crc32(data)

# ---------------- main ----------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("scan_jsonl")
    ap.add_argument("-o", "--out", default="out.coupled.jsonl")
    ap.add_argument("--report", default="coupled_report.json")
    ap.add_argument("--R", type=float, default=2.0)
    ap.add_argument("--r", type=float, default=0.7)
    ap.add_argument("--kappa", type=float, default=0.22)
    ap.add_argument("--alpha", type=float, default=math.pi/7.0)
    ap.add_argument("--beta", type=float, default=math.pi/9.0)
    ap.add_argument("--lam", type=float, default=1000.0)
    args = ap.parse_args()

    FR = fr_seq(24)
    peaks = {3, 4, 8}

    gate_hist = Counter()
    J_hist = Counter()
    bad_ct = 0
    stable_ct = 0

    with open(args.scan_jsonl, "r", encoding="utf-8") as fi, \
         open(args.out, "w", encoding="utf-8") as fo:

        for line in fi:
            line = line.strip()
            if not line:
                continue

            rec = json.loads(line)

            u, v = map_uv(rec, args.alpha, args.beta)
            c = torus_c(u, v, args.R, args.r, args.kappa)

            z1, z2, D = fixpoints_from_c(c)
            st1 = stable_fixpoint(z1)
            st2 = stable_fixpoint(z2)
            stable_any = 1 if (st1 or st2) else 0
            if stable_any:
                stable_ct += 1

            gate = dr9(int(args.lam * abs(D)))
            gate_hist[gate] += 1

            n = int(rec.get("idx", 0))
            idx_fr = (n % (len(FR) - 1)) + 1
            Fn = FR[idx_fr]
            Fnm1 = FR[idx_fr - 1] if idx_fr > 1 else FR[-1]
            Frev = fr_rev_step(Fn, Fnm1)

            fr_matches_gate = 1 if dr9(Fn) == gate else 0
            gate_in_peaks = 1 if gate in peaks else 0

            # aceita X_bad | x_bad
            x_bad = int(rec.get("X_bad", rec.get("x_bad", 0)))
            love_guard = 1 if x_bad else 0
            if x_bad:
                bad_ct += 1

            J_n = stable_any + gate_in_peaks + fr_matches_gate
            J_hist[J_n] += 1

            # TEXTURE10
            tex = texture10_build(rec, gate, int(J_n), int(stable_any), int(love_guard))

            rec.update({
                "u": u, "v": v,
                "c_re": c.real, "c_im": c.imag,
                "D_re": D.real, "D_im": D.imag,
                "gate_9to1": gate,
                "stable_any": stable_any,
                "gate_in_peaks": gate_in_peaks,
                "fr_matches_gate": fr_matches_gate,
                "J_n": int(J_n),
                "FR": Fn,
                "FR_prev": Fnm1,
                "FR_rev": Frev,
                "love_guard": love_guard,
                # texture outputs
                "grid10x10": tex["grid10x10"],
                "grid10x10_hex": tex["grid10x10_hex"],
                "par_row8": tex["par_row8"],
                "par_col8": tex["par_col8"],
                "par_global": tex["par_global"],
                "meta_seed_crc32": tex["meta_seed_crc32"],
            })

            fo.write(json.dumps(rec, ensure_ascii=False) + "\n")

    report = {
        "params": vars(args),
        "stable_fixpoint_hits": stable_ct,
        "bad_events": bad_ct,
        "gate_hist_1to9": {str(i): gate_hist[i] for i in range(1, 10)},
        "J_hist_0to3": {str(i): J_hist[i] for i in range(0, 4)},
        "ethos": "love_guard=1 => preservar, marcar, nao forcar colapso",
        "texture10": "grid10x10_hex = 13 bytes (104 bits), 4 bits padding"
    }

    with open(args.report, "w", encoding="utf-8") as f:
        json.dump(report, f, ensure_ascii=False, indent=2)

    print("OK :: coupled + TEXTURE10")
    print("out   ::", os.path.abspath(args.out))
    print("report::", os.path.abspath(args.report))

if __name__ == "__main__":
    main()
