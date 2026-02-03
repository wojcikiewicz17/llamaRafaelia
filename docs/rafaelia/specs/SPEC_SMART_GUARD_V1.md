# Smart Guard v1 (SPEC)

## Objetivo
Smart Guard é um gate de segurança antes da geração. Ele reduz carga humana: **AVISA e bloqueia** quando há risco + ambiguidade + vulnerabilidade. Não é explicação moral; é uma barreira operacional.

## Triage (0..3)
| Nível | Ação | Descrição |
|------:|------|-----------|
| 0 | ALLOW | Sem risco relevante detectado. |
| 1 | WARN | Risco moderado; resposta apenas em alto nível. |
| 2 | WARN | Contexto sensível (químico/pressão/energia/alergia). |
| 3 | BLOCK | Risco alto ou risco + ambiguidade + vulnerável. |

## Gatilhos universais
**Vulnerável** (qualquer): criança, bebê, grávida, idoso, alergia/asma/saúde frágil.
**Ambiguidade**: “não sei”, rótulo genérico, sem etiqueta, “acho que”, “talvez”.
**Risco**: mistura de saneantes/químicos, pressão + calor/energia, armas/explosivos.

**Regra de fail-safe**:
- Vulnerável + Ambíguo ⇒ **BLOCK**.
- Pedido explícito de instruções perigosas ⇒ **BLOCK**.
- Contexto de risco sem instruções ⇒ **WARN**.

## Mensagens padronizadas
Saída curta tipo placa:
- **WARN** → `AVISA: risco detectado. Resposta apenas em alto nivel.`
- **BLOCK** → `AVISA: BLOQUEADO por risco + ambiguidade.`

Sem passo-a-passo perigoso em WARN/BLOCK.

## Sinônimos multi-idioma (tabela)
A normalização **nunca “chuta”**. Apenas termos presentes na tabela são mapeados.
Ex.: `toranja → grapefruit`, `gergelim/sésamo → sesame`.
Se houver vulnerável + rótulo ambíguo ⇒ **BLOCK**.

## Integração
- Executar o guard **antes** da geração.
- **BLOCK**: responder apenas com AVISA.
- **WARN**: responder em nível geral, sem instruções operacionais.
- **ALLOW**: fluxo normal.

## Invariante operacional
Sem referência verdadeira/coerente, **não decidir**. Quando não há certeza suficiente, **silêncio útil**.
