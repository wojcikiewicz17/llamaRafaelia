# OMEGA :: Command Tree (CLI + NAV)

## Artefatos-base (já gerados)
- omega_msgs.jsonl                  (mensagens extraídas, busca)
- omega_conv_stats.jsonl            (estatística por conversa)
- omega_hot_convs.jsonl             (hot convs filtrado)
- omega_hot_rank.txt                (rank hotspots/densidade)
- omega_threads_rank.txt            (rank threads/profundidade)
- omega_cross_hot_rank.txt          (modelos em hot)
- omega_value_map.jsonl             (valor por conversa)
- omega_value_top                   (ranking C do value_map)
- omega_search_fast                 (busca streaming em C)

## CLI recomendado (omega)
omega A                 -> mostra hot convs rank (omega_hot_rank.txt)
omega B                 -> mostra threads rank   (omega_threads_rank.txt)
omega C                 -> mostra modelos hot    (omega_cross_hot_rank.txt)
omega D <term> [N] [role] -> busca streaming (omega_search_fast ou fallback)

## TUI (omega_nav)
omega_nav               -> menu ncurses
  A  -> Hot convs (densidade)
  B  -> Threads (profundidade)
  C  -> Models em hot
  V  -> Top Valor (infra/sec/ai/symb) via omega_value_top (se existir)
  S  -> Search (prompt interativo)
  H  -> Help
  Q  -> Quit

## Convenções de arquivos (paths)
- assume execução dentro de: ~/rafaelia_triad_lab/cti
- usa arquivos .txt/.jsonl existentes, sem reprocessar o mundo

## Objetivo do NAV
- Navegar 3.572 conversas como um BBS moderno:
  ranking -> drilldown -> busca -> export (futuro)
