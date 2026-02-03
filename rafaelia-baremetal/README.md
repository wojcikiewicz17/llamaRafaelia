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
- **bitstack/**: Low-level non-linear logic operations (NEW)
- **zipraf/**: Archiving and compression utilities
- **rafstore/**: Storage management and data structures
- **toroid/**: Toroidal topology implementations
- **hardware/**: Hardware detection and register access
- **utils/**: Collection of 42 specialized utility tools
- **core/raf_system.{h,c}**: Architecture registry and RF_ID identity map

### NEW: BITSTACK Module

The **bitstack/** module implements non-linear logic operations using bit-stack methodology:

- **Single-Flip Operations**: Core logic based on individual bit flips with cascade effects
- **Opportunity Detection**: Multi-point measurement to identify optimal positions
- **Deterministic Calculations**: Hash-based state tracking for reproducible results
- **Non-Linear Propagation**: State changes that propagate based on current state
- **Iterative Solving**: Reach target states through optimized flip sequences

See [bitstack/README.md](bitstack/README.md) for complete documentation.

### The 42 Utility Tools

The utils module provides 42 specialized baremetal tools organized into categories:

1. **Memory Operations (Tools 1-5)**: Secure comparison, zeroing, safe copy, swap, pattern fill
2. **String Operations (Tools 6-10)**: Safe length, copy, concatenation, case-insensitive compare, tokenization
3. **Math Operations (Tools 11-15)**: Integer sqrt, power, GCD, LCM, modular exponentiation
4. **Bit Operations (Tools 16-20)**: Popcount, find first/last set, rotate left/right
5. **Checksums/Hashing (Tools 21-25)**: Fletcher-16/32, Adler-32, MurmurHash3, FNV-1a
6. **Encoding/Decoding (Tools 26-30)**: Base64, hexadecimal, URL encoding
7. **Random Generation (Tools 31-35)**: PRNG initialization, generation, range, float, bytes
8. **Sorting/Searching (Tools 36-40)**: Quicksort, binary search, insertion sort, min/max
9. **Time/Profiling (Tools 41-42)**: High-resolution timestamp, microsecond delay

See [docs/42_TOOLS.md](docs/42_TOOLS.md) for detailed documentation of all tools.

## Building

The baremetal module can be built independently or as part of the llama.cpp project:

```bash
# Build with baremetal support (portable by default)
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release

# Run tests
./build/bin/rafaelia-test

# Run basic examples
./build/bin/rafaelia-example

# Run comprehensive integration examples
./build/bin/rafaelia-integration
```

### Build Options

#### Portable vs Native Optimization

By default, the module builds with portable optimizations that work across different CPUs of the same architecture family. For maximum performance on a specific machine, you can enable native CPU optimizations:

```bash
# Portable build (default - recommended for distribution)
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release

# Native optimization build (maximum performance, non-portable)
cmake -B build -DRAFAELIA_BAREMETAL=ON -DRAFAELIA_NATIVE_OPTIMIZE=ON
cmake --build build --config Release
```

**Important**: Native optimization uses `-march=native` and `-mtune=native` flags, which optimize for the build machine's CPU but may not run on other CPUs. Use portable builds for:
- Cross-compilation
- Container images
- Binary distribution
- Academic/research environments with diverse hardware

## Platform Compatibility

The Rafaelia Baremetal Module is designed to be highly portable across different platforms and architectures:

### Supported Architectures

- **x86**: 32-bit x86 (i386, i686)
- **x86-64**: 64-bit x86 (AMD64, Intel 64)
- **ARM**: 32-bit ARM (ARMv7, ARMv8 32-bit mode)
- **ARM64**: 64-bit ARM (AArch64, Apple Silicon)
- **RISC-V**: 32-bit and 64-bit RISC-V
- **MIPS**: MIPS architecture
- **PowerPC**: PowerPC architecture

### Supported Operating Systems

- **Linux**: All major distributions (Ubuntu, Debian, Fedora, RHEL, Arch, etc.)
- **macOS**: Intel and Apple Silicon
- **Windows**: Windows 7+ (MSVC, MinGW, Cygwin)
- **BSD**: FreeBSD, NetBSD, OpenBSD
- **Unix-like**: Any POSIX-compliant system

### Hardware Detection

The module includes comprehensive hardware detection that automatically identifies:
- CPU architecture and features (SSE, AVX, NEON, SVE, etc.)
- Number of physical CPU cores
- Cache line sizes
- Platform-specific optimizations

Hardware detection works correctly on all supported platforms and gracefully falls back to safe defaults on unknown systems.

### Compatibility Features

1. **No External Dependencies**: Pure C11 implementation with only standard library
2. **Portable Compilation**: Default build works across different CPUs of same architecture
3. **Cross-Platform APIs**: Consistent API across all platforms
4. **Standard Headers**: Uses only `stdint.h`, `unistd.h` (POSIX), `windows.h` (Windows)
5. **Fallback Mechanisms**: Safe defaults when hardware detection unavailable

## Usage

### Quick Start

```c
#include "rafaelia_baremetal.h"

int main(void) {
    // Detect hardware
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    
    // Create matrix
    raf_matrix *m = raf_matrix_create(3, 3);
    raf_matrix_identity(m);
    
    // Use utilities
    raf_prng prng;
    raf_prng_init(&prng, 12345);
    uint64_t random = raf_prng_next(&prng);
    
    // Cleanup
    raf_matrix_destroy(m);
    return 0;
}
```

### Architecture Registry (RF_ID)

The system registry exposes the module hierarchy described in the architecture diagram:

```c
#include "core/raf_system.h"

const raf_system_identity *identity = raf_system_identity_get();
printf("RF_ID: %s\\n", identity->rf_id);

const raf_module_info *modules = NULL;
unsigned int count = raf_system_list_modules(&modules);
```

### Documentation

- **[Complete API Guide](docs/API_GUIDE.md)** - Comprehensive examples for all modules
- **[42 Tools Documentation](docs/42_TOOLS.md)** - Detailed utility tools reference
- **[Technical Specifications](docs/TECHNICAL.md)** - Architecture and implementation details
- **[Summary](docs/SUMMARY.md)** - High-level overview

See also:
- `example.c` - Basic usage examples for each module
- `integration_example.c` - Real-world integration scenarios
- Individual module READMEs in component directories

## Design Principles

1. **No External Dependencies**: Only standard C library functions
2. **Deterministic Behavior**: Same input always produces same output
3. **Hardware-Specific**: Optimized for different CPU architectures
4. **Matrix Operations**: Linear algebra using native arrays
5. **Minimal Overhead**: Direct memory access and register operations
