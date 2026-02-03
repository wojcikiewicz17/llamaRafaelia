# Rafaelia Additions

This folder contains all Rafaelia-specific documentation, specifications, and reports that extend the upstream **llama.cpp** baseline. The goal is to keep the repository root upstream-friendly while making the Rafaelia layer easy to find, audit, and sync.

## Structure

```
docs/rafaelia/
├── README.md
├── LICENSING.md
├── UPSTREAM_TRACKING.md
├── design/
├── reports/
├── specs/
└── tests/
```

## Key Documents

- **Specs**
  - [BitStack World Model v1](specs/SPEC_BITSTACK_WORLD_MODEL_V1.md)
  - [Smart Guard v1](specs/SPEC_SMART_GUARD_V1.md)
- **Design**
  - [Rafaelia architecture](design/LLAMA_RAFAELIA_DESIGN.md)
  - [Implementation details](design/RAFAELIA_IMPLEMENTATION.md)
- **Reports**
  - [HD cache report](reports/HD_CACHE_IMPLEMENTATION_REPORT.md)
  - [Cleanup guide](reports/CLEANUP_GUIDE.md)
- **Tests**
  - [Smart Guard tests](tests/SMART_GUARD_TESTS.md)
- **Upstream tracking**
  - [UPSTREAM_TRACKING.md](UPSTREAM_TRACKING.md)
- **Licensing**
  - [LICENSING.md](LICENSING.md)

## Build Notes

- Rafaelia Baremetal is **opt-in**. Enable with `-DRAFAELIA_BAREMETAL=ON`.
- Smart Guard is controlled via flags:
  - `--smart-guard on|off`
  - `--smart-guard-policy <path>`
  - `--smart-guard-mode warn|block`
  - `--smart-guard-log <path>`
- Rafstore prefetch options (server/CLI shared):
  - `--rafstore-cache-dir <dir>`
  - `--rafstore-prefetch-window <mb>`
  - `--rafstore-prefetch-strategy <name>`
