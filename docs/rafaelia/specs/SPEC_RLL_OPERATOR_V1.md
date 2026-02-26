# RLL Operator v1 (SPEC)

## Objetivo
Definir o contrato operacional mínimo do template `models/templates/rafaelia-rll.jinja`: toda resposta do assistente deve vir em quatro blocos fixos para facilitar validação manual e automação.

## Contrato obrigatório de saída
Toda resposta deve conter **exatamente** estes blocos, nesta ordem:
1. `CONTEXT`
2. `STATE`
3. `TRANSITION`
4. `RESULT`

Regras:
- Não renomear títulos.
- Não remover blocos.
- Não inserir bloco adicional.
- O conteúdo pode variar, mas os quatro blocos devem sempre existir.

## Exemplo concreto (prompt renderizado)
Exemplo de entrada (mensagens):
- system: `Modo operador RLL. Responder curto.`
- user: `Qual o próximo passo para validar o formato?`

Prompt final renderizado pelo template:

```text
<s>System: Operador RLL obrigatório. Em TODA resposta, use exatamente os quatro blocos abaixo, nesta ordem e com estes títulos literais: CONTEXT, STATE, TRANSITION, RESULT. Não omita blocos, não renomeie blocos e não adicione blocos extras.
System: Modo operador RLL. Responder curto.
User: Qual o próximo passo para validar o formato?
Assistant:
```

## Resposta esperada (formato)
Exemplo válido mínimo:

```text
CONTEXT
Validação manual do contrato de saída RLL em execução.

STATE
Template ativo e instrução de quatro blocos presente no bloco de sistema.

TRANSITION
Executar uma pergunta simples no llama-cli/llama-server e verificar presença e ordem dos quatro títulos.

RESULT
Formato confirmado: CONTEXT → STATE → TRANSITION → RESULT.
```

## Checklist manual (`llama-cli` / `llama-server`)
- Renderizar prompt com `--chat-template-file models/templates/rafaelia-rll.jinja`.
- Enviar pergunta curta.
- Validar que a resposta contém os quatro títulos na ordem correta.
- Reprovar caso falte, troque nome, ou inclua bloco extra.
# SPEC: RLL Operator v1 (Llama Prompt Layer)

## Objective

Apply the RAFAELIA **Relativity Living-Light (RLL)** symbolic flow as a deterministic prompt layer on top of llama.cpp runtime, without adding external dependencies.

## Scope

- Input/output prompt structure only.
- No model-architecture changes.
- Works in `llama-cli` and `llama-server` via `--chat-template-file`.

## Core Mapping

The operator maps conversational input into four deterministic blocks:

1. `CONTEXT` — raw user intention and constraints.
2. `STATE` — current symbolic state over the BIT Ω lattice.
3. `TRANSITION` — deterministic update rule used for this turn.
4. `RESULT` — final response and next-state hint.

### BIT Ω 10-state lattice (v1)

`{-1, -0, 0, +0, +1, ±, Ø, Ω, Δ, Σ}`

- `-1`: strong negation / reject.
- `-0`: weak negation / damp.
- `0`: neutral / idle.
- `+0`: weak activation.
- `+1`: strong activation.
- `±`: bifurcation / branch.
- `Ø`: out-of-field / undefined.
- `Ω`: saturation / boundary.
- `Δ`: transition in progress.
- `Σ`: aggregated closure.

## Unicode Integrity

The operator preserves literal symbols exactly:

`⊕ ⊗ ∮ ∫ √ π φ Δ Ω Σ ψ χ ρ ∧`

No normalization or symbol replacement is allowed in prompt serialization.

## Runtime Integration

Template file:

- `models/templates/rafaelia-rll.jinja`

Example (`llama-cli`):

```bash
./build/bin/llama-cli \
  -m ./models/model.gguf \
  --jinja \
  --chat-template-file models/templates/rafaelia-rll.jinja \
  -p "aplicar no llama mantendo Ω e Δ"
```

Example (`llama-server`):

```bash
./build/bin/llama-server \
  -m ./models/model.gguf \
  --jinja \
  --chat-template-file models/templates/rafaelia-rll.jinja
```

## Expected Behavior

- System prompt defaults to RLL deterministic constraints when not explicitly provided.
- User and assistant turns are serialized with fixed tags:
  - `[USER]...[/USER]`
  - `[ASSISTANT]...[/ASSISTANT]`
- Generation prompt opens with `[ASSISTANT]` when `add_generation_prompt=true`.

## Non-goals (v1)

- No token-level reweighting.
- No custom sampler changes.
- No backend-specific kernels.
