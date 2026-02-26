# Licensing Overview

- **Upstream license:** llama.cpp is MIT licensed (see repository `LICENSE`).
- **Rafaelia Baremetal:** MIT licensed (see `rafaelia-baremetal/LICENSE`).

## Guidance

- Rafaelia additions are intended to be MIT-compatible and do not change the licensing of upstream code.
- The Rafaelia Baremetal library is linked optionally via build flags, so downstream users can consume upstream behavior without additional requirements.
- When distributing binaries that include the Rafaelia Baremetal library, include both MIT license texts (upstream and Rafaelia Baremetal).

## Add-on Licensing Nucleus (Low-level)

A deterministic licensing nucleus for add-ons is available in:

- Header: `rafaelia-baremetal/licensing/raf_licensing.h`
- Source: `rafaelia-baremetal/licensing/raf_licensing.c`

It provides fixed-size C structs and validation routines for interoperability with normative references such as:

- Berne Convention (`RAF_LICENSE_NORM_BERNE_CONVENTION`)
- Montevideo Convention (`RAF_LICENSE_NORM_MONTEVIDEO_CONVENTION`)
- UDHR / UN Charter / UNESCO / UNICEF references
- Traditional knowledge, faith/sacred works, and local custom frameworks

The nucleus does not replace legal counsel; it standardizes metadata representation for compliance-aware add-ons.
