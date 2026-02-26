PIPELINE CORRETO (TERMUX / CELULAR)

1) Scan CTI (argumentos posicionais!)
   ./rafa_cti_scan input.zip out.bitstack.jsonl --csv out.csv

2) Acoplar TRIAD + Amor
   python3 triad_cti_couple.py out.bitstack.jsonl \
     -o out.coupled.jsonl \
     --report coupled_report.json

DICA:
- Sempre apague out.bitstack.jsonl antes de novo scan
- Se X_bad == 1 -> love_guard == 1 (esperado)
