# Upstream Tracking (llama.cpp)

- **upstream_repo:** ggml-org/llama.cpp
- **upstream_base_commit:** f673f0d67488d9f7c89f0fa077b1f2f3086f7457
- **rafaelia_patchset:** rafaelia-layer-1
- **sync_policy:** rebase monthly (or after major upstream releases)

## Merge Notes

- Keep upstream core structure intact: `build/`, `src/`, `common/`, `tools/`, `docs/`, `ggml/`.
- Rafaelia docs live under `docs/rafaelia/` to minimize merge conflicts.
- Rafaelia Baremetal is opt-in via `-DRAFAELIA_BAREMETAL=ON` and only links to `tools/` targets.

## Differences from Upstream

- **Rafaelia documentation tree** in `docs/rafaelia/` (specs, design, reports, tests).
- **Smart Guard** safety gate wired into `llama-cli` and `llama-server` with shared metadata schema and configurable policy/mode/log flags.
- **Rafaelia Baremetal** optional module wired into tools with a minimal integration bridge (`rafaelia.h`).
- **Rafstore prefetch hook** executed before server model load (opt-in via build flag).
- **Q4 witness experiment** parked in `pocs/witness_q4/` to avoid unused core build artifacts.
