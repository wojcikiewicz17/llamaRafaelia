# Rafaelia Baremetal Module - Technical Documentation

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Core Components](#core-components)
3. [Building and Integration](#building-and-integration)
4. [API Reference](#api-reference)
5. [Performance Considerations](#performance-considerations)
6. [Hardware Support](#hardware-support)

## Architecture Overview

The Rafaelia Baremetal Module is designed as a self-contained, dependency-free library for high-performance computing operations. The module is structured into six main components:

### Component Hierarchy

```
rafaelia-baremetal/
├── core/          - Linear algebra and matrix operations
├── bitraf/        - Bit-level operations and compression
├── zipraf/        - Archiving and data compression
├── rafstore/      - Storage management and data structures
├── toroid/        - Toroidal topology implementations
└── hardware/      - Hardware detection and CPU features
```

## Core Components

### 1. Core Module (`core/`)

Implements deterministic linear algebra operations using matrix arrays.

**Key Features:**
- Matrix creation and manipulation
- Deterministic operations using Kahan summation
- Vector operations (dot product, norm)
- Matrix multiplication with double-precision accumulation
- Element-wise operations

**Example Usage:**
```c
#include "core/raf_core.h"

raf_matrix *a = raf_matrix_create(4, 4);
raf_matrix *b = raf_matrix_create(4, 4);
raf_matrix *result = raf_matrix_create(4, 4);

raf_matrix_identity(a);
raf_matrix_fill(b, 2.0f);
raf_matrix_mul(a, b, result);

raf_matrix_destroy(a);
raf_matrix_destroy(b);
raf_matrix_destroy(result);
```

### 2. BITRAF Module (`bitraf/`)

Provides bit-level operations and compression utilities.

**Key Features:**
- Bit stream operations (read/write individual bits)
- Bit manipulation utilities (count, reverse, etc.)
- Bit packing for quantization (4-bit, 2-bit)
- Delta encoding for sequential data
- Run-length encoding

**Example Usage:**
```c
#include "bitraf/raf_bitraf.h"

raf_bitstream *bs = raf_bitstream_create(1024);
raf_bitstream_write_bits(bs, 0xABCD, 16);

uint32_t value;
raf_bitstream_reset(bs);
raf_bitstream_read_bits(bs, &value, 16);

raf_bitstream_destroy(bs);
```

### 3. ZIPRAF Module (`zipraf/`)

Implements data archiving and compression.

**Key Features:**
- Archive creation and management
- CRC32 checksums for integrity
- LZ77 compression (framework for implementation)
- Huffman coding (framework for implementation)
- Entry-based archive format

**Example Usage:**
```c
#include "zipraf/raf_zipraf.h"

raf_zipraf_archive *archive = raf_zipraf_create();
uint8_t data[] = {1, 2, 3, 4, 5};
raf_zipraf_add_data(archive, "test.dat", data, 5);

uint8_t *output;
size_t size;
raf_zipraf_extract_data(archive, "test.dat", &output, &size);

free(output);
raf_zipraf_destroy(archive);
```

### 4. RAFSTORE Module (`rafstore/`)

Provides efficient storage management and data structures.

**Key Features:**
- Memory pool management
- Ring buffer for streaming data
- Key-value store with hash table
- LRU cache implementation
- Hash functions for efficient lookup

**Example Usage:**
```c
#include "rafstore/raf_rafstore.h"

raf_kv_store *store = raf_kv_create(256);
int value = 42;
raf_kv_set(store, "my_key", &value, sizeof(value));

void *retrieved;
raf_kv_get(store, "my_key", &retrieved, NULL);

raf_kv_destroy(store);
```

### 5. TOROID Module (`toroid/`)

Implements toroidal topology structures for spatial operations.

**Key Features:**
- 2D and 3D toroidal grids
- Automatic coordinate wrapping
- Neighbor access (8-connected for 2D, 26-connected for 3D)
- Toroidal distance calculations
- Diffusion simulation
- Cellular automata support
- Convolution with toroidal wrapping

**Example Usage:**
```c
#include "toroid/raf_toroid.h"

raf_toroid_2d *toroid = raf_toroid_2d_create(100, 100);
raf_toroid_2d_set(toroid, 50, 50, 1.0f);

// Wrapping works automatically
float value = raf_toroid_2d_get(toroid, -1, -1);  // Gets from (99, 99)

raf_toroid_2d_destroy(toroid);
```

### 6. Hardware Module (`hardware/`)

Detects hardware capabilities and provides CPU feature information.

**Key Features:**
- Architecture detection (x86, ARM, RISC-V, etc.)
- CPUID support for x86/x64
- CPU feature detection (SSE, AVX, NEON, etc.)
- Cache information
- Core count detection

**Example Usage:**
```c
#include "hardware/raf_hardware.h"

raf_cpu_features features;
raf_detect_cpu_features(&features);

if (features.has_avx2) {
    // Use AVX2-optimized code path
}
```

## Building and Integration

### Standalone Build

```bash
cd rafaelia-baremetal
mkdir build
cd build
cmake ..
cmake --build .
```

### Integration with llama.cpp

Add to the root `CMakeLists.txt`:

```cmake
# Add Rafaelia Baremetal Module
option(RAFAELIA_BAREMETAL "Build with Rafaelia Baremetal support" OFF)

if(RAFAELIA_BAREMETAL)
    add_subdirectory(rafaelia-baremetal)
    target_link_libraries(llama PRIVATE rafaelia-baremetal)
endif()
```

Build with baremetal support:

```bash
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release
```

## Performance Considerations

### Deterministic Operations

All operations in the core module use deterministic algorithms:
- Kahan summation for floating-point accumulation
- Fixed order of operations in matrix multiplication
- Double-precision intermediate results

This ensures reproducible results across different platforms and compilation settings.

### Memory Management

- Use memory pools (`rafstore/`) for frequent allocations
- Ring buffers for streaming data minimize overhead
- Cache structures improve lookup performance

### Hardware Optimization

- Detect CPU features at runtime
- Use appropriate SIMD instructions when available
- Align data structures to cache line boundaries

## Hardware Support

### Supported Architectures

- x86 (32-bit and 64-bit)
- ARM (32-bit and 64-bit / AArch64)
- RISC-V (32-bit and 64-bit)
- MIPS
- PowerPC

### CPU Features Detected

**x86/x64:**
- SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2
- AVX, AVX2, AVX-512
- FMA (Fused Multiply-Add)

**ARM:**
- NEON
- SVE (Scalable Vector Extension)

## Future Enhancements

The module is designed to be extended with:
1. Additional 42 utility tools as specified in requirements
2. ASM optimizations for critical paths
3. Advanced compression algorithms (full LZ77, Huffman)
4. GPU acceleration interfaces
5. Distributed computing support via toroidal topology

## License

This module is licensed under the MIT License. See the LICENSE file for details.

Copyright (c) 2026 Rafael Melo Reis
