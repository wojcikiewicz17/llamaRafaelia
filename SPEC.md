# BitStack World Model v1 (SPEC)

## Definições
- **Matéria (nibbles/bytes)**: o substrato físico-lógico. Nibbles e bytes são tratados como unidades de matéria que, quando organizadas, formam blocos e mundos.
- **Leis (Witness)**: regras de validação por bloco. Um bloco só é considerado utilizável quando a sua verificação (Witness) é verdadeira.
- **Memória (base + overlays/layers iguais)**: o armazenamento é composto por uma base imutável e overlays/layers equivalentes; todos os layers seguem o mesmo esquema de blocos.
- **Testemunha (validação)**: processo explícito que marca cada bloco como válido (Witness=true) ou inválido (Witness=false).

## Invariantes
- **Nenhuma computação usa bloco com `Witness=false`**.
- A coerência operacional segue um eixo imutável (verdade/amor/bem) como “north star”.

## Layout por bloco
- **Planar (HI/LO/CHK)**: bloco dividido em camadas High/Low + checksum/validação.
- **Q4 planar (W/CHK/META)**: bloco dividido em dados quantizados (W), verificação (CHK) e metadados (META).

## Política de integridade
- **Detect-only**: ao falhar Witness, o bloco é marcado como inválido e não é usado.
- **Recover-1**: se houver paridade/duplicação, tenta-se recuperar um bloco; se falhar, aplica-se detect-only.

## Warmup/heat
- **Pre-touch**: tocar previamente a memória para reduzir variância térmica/latência.
- **LUT warm**: aquecer tabelas (lookup tables) críticas antes da execução para estabilidade.

## Notas de escopo
- O modelo opera com vetores-direção em matriz (não bits isolados).
- O eixo de coerência é tratado como requisito operacional, não heurística.
