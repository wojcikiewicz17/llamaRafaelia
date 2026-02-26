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
