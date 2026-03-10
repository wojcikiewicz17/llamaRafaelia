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


PIPELINE OMEGA (ordem completa de artefatos)
0) (se necessário) reconstruir objetos-base
   ./omega_json_reconstruct ~/storage/downloads/conversations.json omega_objs.jsonl --progress

1) Preparar mensagens (run_omega_pipeline faz isso automaticamente se faltar omega_msgs.jsonl)
   python3 omega_extract_msgs.py --in_objs omega_objs.jsonl --out omega_msgs.jsonl

2) Rodar pipeline de zonas (aceita argumentos; defaults preservados)
   ./run_omega_pipeline.sh [omega_zone_metrics.jsonl] [omega_msgs.jsonl] [omega_metrics_v3.jsonl]

Cadeia de dependências (evita execução fora de ordem):
- omega_objs.jsonl -> omega_msgs.jsonl -> omega_zone_metrics.jsonl + omega_metrics_v3.jsonl (via ./ozp)
