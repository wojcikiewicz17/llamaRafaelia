# BitStack World Model v1 (SPEC)

## Objetivo
BitStack é um armazenamento em memória, bit-addressable e zero-copy. Não há “descompressão” no sentido clássico (zlib/deflate): o BitStack é consumido inteiro, com camadas (overlays) de mesma geometria e layout. O que muda é o mapa/overlay, não a forma.

## Estrutura do mundo
**Mundo = (Matéria, Leis, Memória, Testemunha)**.
- **Matéria**: bytes/nibbles/bitplanes como substrato físico-lógico.
- **Leis (Witness)**: verificação por bloco. Um bloco só é válido se Witness=true.
- **Memória**: base + overlays/layers com layout idêntico (mesma forma).
- **Testemunha**: marcação explícita do estado do bloco (válido/inválido).

### Invariante fundamental
Nenhuma computação consome bloco com `Witness=false`.

## Layout e overlays
- **Planar por nibble**: 1 byte = HI/LO em planos separados. Mantém localidade e permite checagem rápida.
- **Overlays iguais**: todos os layers seguem a mesma geometria. Troca-se o overlay (mapa), não a estrutura.
- **Zero-copy**: consumo direto do layout, sem reempacotar ou “descomprimir”.

## Warmup / Heat
Antes da execução:
- **Pre-touch de páginas** para reduzir page faults e variância de latência.
- **Aquecimento de LUT** para estabilizar o hot path.

## Performance (hot path)
- Evitar branch por elemento; preferir **dispatch por bloco**.
- Checagem por bloco, cacheline-friendly.
- “Flip out”: substituir branch por máscara/cmov.
- Otimizar layout (SoA/planar) e alinhamento.

## ECC geométrico (ASCII/matéria)
- Organizar bytes/nibbles em **grid 4×4** para redundância geométrica.
- Simetrias: linhas/colunas/diagonais/anéis/rotações.
- “1.5 bit” representa **densidade efetiva** via bitplane + mask/overlay.

## Split de nibbles e verificação
- **Nibble split**: 1 byte = HI/LO em planos distintos.
- **CHK simples**: `CHK = HI XOR LO` para detectar “texto quebrado”.
- **Recover-1 opcional**: se houver duplicação/paridade, tentar recuperar; caso contrário, **detect-only**.

## Integração com LLaMA (pesos quantizados)
- Armazenar **nibbles e metadados** em planos separados.
- **Witness por BLOCO** (nunca por peso individual).
- Check por bloco via XOR-fold ou CRC32C.
- Fallback por bloco: **neutralizar/zerar/pular** (sem usar bloco inválido).

## Política de integridade
- **Detect-only**: se Witness falhar, bloco é marcado inválido.
- **Recover-1**: só quando há redundância explícita.

## Resumo operacional
- BitStack é inteiro, planar e zero-copy.
- Overlays têm a mesma forma; só o mapa muda.
- Warmup estabiliza throughput.
- Witness por bloco é a fronteira de segurança do mundo.
