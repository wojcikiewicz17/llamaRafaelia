#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import csv, sys, math

def main():
    if len(sys.argv) < 2:
        print("usage: analyze_trace.py path/to/triad_trace.csv", file=sys.stderr)
        return 2

    path = sys.argv[1]
    rows = 0
    esc = 0
    stable = 0

    peaks_total = 0
    peaks_stable = 0
    non_total = 0
    non_stable = 0

    j_hist = [0,0,0,0]

    with open(path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            rows += 1
            e = int(row.get("escaped","0"))
            esc += e

            st = int(row.get("stable_any","0"))
            stable += st

            gate = int(row.get("gate_9to1", row.get("gate","0")))
            if gate in (3,4,8):
                peaks_total += 1
                peaks_stable += st
            else:
                non_total += 1
                non_stable += st

            j = int(row.get("J_n","0"))
            if 0 <= j <= 3:
                j_hist[j] += 1

    def p(a,b): return (a/b) if b else float("nan")
    p_peaks = p(peaks_stable, peaks_total)
    p_non   = p(non_stable, non_total)
    delta   = p_peaks - p_non if (math.isfinite(p_peaks) and math.isfinite(p_non)) else float("nan")

    # CSV line (no header)
    # rows,esc,stable_rate,p_peaks,p_non,delta,j0,j1,j2,j3
    stable_rate = p(stable, rows)

    print(",".join([
        str(rows),
        str(esc),
        f"{stable_rate:.6f}",
        f"{p_peaks:.6f}" if math.isfinite(p_peaks) else "nan",
        f"{p_non:.6f}"   if math.isfinite(p_non) else "nan",
        f"{delta:.6f}"   if math.isfinite(delta) else "nan",
        str(j_hist[0]), str(j_hist[1]), str(j_hist[2]), str(j_hist[3]),
    ]))

if __name__ == "__main__":
    raise SystemExit(main())
