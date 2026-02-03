# Licensing Overview

- **Upstream license:** llama.cpp is MIT licensed (see repository `LICENSE`).
- **Rafaelia Baremetal:** MIT licensed (see `rafaelia-baremetal/LICENSE`).

## Guidance

- Rafaelia additions are intended to be MIT-compatible and do not change the licensing of upstream code.
- The Rafaelia Baremetal library is linked optionally via build flags, so downstream users can consume upstream behavior without additional requirements.
- When distributing binaries that include the Rafaelia Baremetal library, include both MIT license texts (upstream and Rafaelia Baremetal).
