# Rafaelia Baremetal Module

## Overview

This module contains custom baremetal implementations designed for deterministic linear algebra operations with minimal dependencies. All code in this module is pure C and Assembly (when necessary) without relying on external libraries.

## Philosophy

The Rafaelia Baremetal Module focuses on:

1. **Baremetal Computing**: Pure C/ASM implementations without library dependencies
2. **Deterministic Operations**: Linear algebra operations with predictable, reproducible results
3. **Matrix-Based Variables**: Using array-based matrices for coherent mathematical operations
4. **Hardware Awareness**: Direct register access and hardware-specific optimizations

## License

This module is created as an independent extension to llama.cpp and maintains compatibility with the MIT license of the original project. All modifications are contained within this separate module to respect the authorship and licensing of the original llama.cpp project.

### Original llama.cpp License

llama.cpp is licensed under the MIT License:
- Copyright (c) 2023-2024 The ggml authors
- See the LICENSE file in the root directory

### Rafaelia Baremetal Module

This module contains original implementations by Rafael Melo Reis and is also provided under the MIT License to maintain compatibility.

## Architecture

### Core Components

- **core/**: Fundamental baremetal operations and utilities
- **bitraf/**: Bit-level operations and compression
- **zipraf/**: Archiving and compression utilities
- **rafstore/**: Storage management and data structures
- **toroid/**: Toroidal topology implementations
- **hardware/**: Hardware detection and register access

## Building

The baremetal module can be built independently or as part of the llama.cpp project:

```bash
# Build with baremetal support
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release
```

## Usage

See individual component documentation in their respective directories.

## Design Principles

1. **No External Dependencies**: Only standard C library functions
2. **Deterministic Behavior**: Same input always produces same output
3. **Hardware-Specific**: Optimized for different CPU architectures
4. **Matrix Operations**: Linear algebra using native arrays
5. **Minimal Overhead**: Direct memory access and register operations
