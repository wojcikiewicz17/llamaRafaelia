#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
python3 src/triad_engine.py --steps 300 --seed 42
