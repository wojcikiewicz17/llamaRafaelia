# Rafaelia Baremetal Module - Compatibility Guide

## Overview

The Rafaelia Baremetal Module is designed with maximum portability and compatibility in mind. This document describes the supported platforms, architectures, and compatibility features.

## Cross-Platform Compatibility

### Supported Architectures

The module includes comprehensive architecture detection and support for:

| Architecture | 32-bit | 64-bit | Status | Notes |
|--------------|--------|--------|--------|-------|
| **x86** | ✅ i386, i686 | ✅ x86-64, AMD64 | Fully Supported | SSE/AVX detection included |
| **ARM** | ✅ ARMv7 | ✅ AArch64 | Fully Supported | NEON/SVE detection included |
| **RISC-V** | ✅ RV32 | ✅ RV64 | Fully Supported | Standards-compliant |
| **MIPS** | ✅ | ✅ | Supported | Basic support |
| **PowerPC** | ✅ | ✅ | Supported | Basic support |

### Supported Operating Systems

| Operating System | Status | Notes |
|------------------|--------|-------|
| **Linux** | ✅ Fully Supported | All major distributions |
| **macOS** | ✅ Fully Supported | Intel and Apple Silicon |
| **Windows** | ✅ Fully Supported | Win7+, MSVC/MinGW/Cygwin |
| **FreeBSD** | ✅ Fully Supported | POSIX-compliant |
| **NetBSD** | ✅ Fully Supported | POSIX-compliant |
| **OpenBSD** | ✅ Fully Supported | POSIX-compliant |
| **Other Unix** | ✅ Supported | Any POSIX system |

### Compiler Compatibility

| Compiler | Minimum Version | Status |
|----------|-----------------|--------|
| **GCC** | 4.9+ | ✅ Fully Supported |
| **Clang** | 3.9+ | ✅ Fully Supported |
| **MSVC** | VS2015+ | ✅ Fully Supported |
| **Intel ICC** | 17.0+ | ✅ Compatible |
| **MinGW** | 5.0+ | ✅ Fully Supported |

## Build Configuration

### Portable vs Native Builds

The module offers two build modes to balance portability and performance:

#### Portable Mode (Default)

```bash
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release
```

**Characteristics:**
- Uses `-O3` optimization without architecture-specific flags
- Binaries work across different CPUs of the same architecture family
- Recommended for:
  - Binary distribution
  - Container images (Docker, Podman, etc.)
  - Cross-compilation
  - Academic/research environments
  - CI/CD pipelines
  - Heterogeneous computing clusters

**Performance:**
- Good optimization without CPU-specific instructions
- May be 5-15% slower than native builds on modern CPUs
- Better compatibility across deployment targets

#### Native Mode (Opt-in)

```bash
cmake -B build -DRAFAELIA_BAREMETAL=ON -DRAFAELIA_NATIVE_OPTIMIZE=ON
cmake --build build --config Release
```

**Characteristics:**
- Uses `-march=native -mtune=native` for maximum performance
- Binaries optimized for build machine's specific CPU
- Recommended for:
  - Local development
  - Performance benchmarking
  - Single-machine deployments
  - Known homogeneous hardware

**Performance:**
- Maximum performance using CPU-specific instructions
- 5-15% faster than portable builds
- May not run on different CPUs (illegal instruction errors)

## Hardware Detection Features

### CPU Architecture Detection

The module automatically detects the CPU architecture at compile time and runtime:

```c
#include "hardware/raf_hardware.h"

raf_arch_type arch = raf_detect_arch();
printf("Architecture: %s\n", raf_arch_name(arch));
```

Detected architectures:
- `RAF_ARCH_X86_32` - 32-bit x86
- `RAF_ARCH_X86_64` - 64-bit x86
- `RAF_ARCH_ARM_32` - 32-bit ARM
- `RAF_ARCH_ARM_64` - 64-bit ARM (AArch64)
- `RAF_ARCH_RISCV_32` - 32-bit RISC-V
- `RAF_ARCH_RISCV_64` - 64-bit RISC-V
- `RAF_ARCH_MIPS` - MIPS
- `RAF_ARCH_POWERPC` - PowerPC
- `RAF_ARCH_UNKNOWN` - Unknown (safe fallback)

### CPU Feature Detection

The module detects CPU-specific features:

```c
raf_cpu_features features;
raf_detect_cpu_features(&features);

if (features.has_avx2) {
    printf("AVX2 available\n");
}
```

**x86/x64 features:**
- SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2
- AVX, AVX2, AVX-512
- FMA (Fused Multiply-Add)

**ARM features:**
- NEON (Advanced SIMD)
- SVE (Scalable Vector Extension)

### Core Count Detection

Proper CPU core detection across platforms:

```c
int cores = raf_get_num_cores();
printf("CPU cores: %d\n", cores);
```

**Implementation:**
- **Windows**: Uses `GetSystemInfo()` API
- **Linux/macOS/Unix**: Uses `sysconf(_SC_NPROCESSORS_ONLN)`
- **BSD**: Uses `sysconf()` with POSIX standard
- **Fallback**: Returns 1 core (safe default)

### Cache Information

Basic cache size detection:

```c
unsigned int cache_line = raf_get_cache_line_size();
unsigned int l1_size = raf_get_l1_cache_size();
unsigned int l2_size = raf_get_l2_cache_size();
unsigned int l3_size = raf_get_l3_cache_size();
```

**Implementation:**
- **x86/x64**: Uses CPUID instruction for accurate detection
- **ARM**: Uses typical values (64-byte cache lines)
- **Other**: Uses safe defaults

## Standard Library Dependencies

### Required Headers

The module uses only standard C11 headers:

- `stdint.h` - Standard integer types (uint32_t, int64_t, etc.)
- `stdlib.h` - Memory allocation (malloc, free)
- `string.h` - String operations (memcpy, memset)
- `math.h` - Mathematical functions (sqrt, sin, cos)

### Platform-Specific Headers

Used only when needed and properly guarded:

- `unistd.h` - POSIX systems (Linux, macOS, BSD) for `sysconf()`
- `windows.h` - Windows for `GetSystemInfo()`
- `intrin.h` - MSVC for CPUID intrinsics

All platform-specific code is properly guarded with preprocessor directives.

## Interoperability

### C/C++ Compatibility

All headers include C++ guards:

```c
#ifdef __cplusplus
extern "C" {
#endif

// API declarations

#ifdef __cplusplus
}
#endif
```

This allows the module to be used from both C and C++ code.

### ABI Stability

- Uses C linkage for all public APIs
- Fixed struct layouts (no padding differences)
- Consistent calling conventions
- No C++ exceptions or RTTI

### Academic and Research Environments

The module is designed for academic use cases:

1. **Reproducibility**: Deterministic operations with consistent results
2. **Portability**: Works across diverse university computing clusters
3. **Education**: Clear, readable code for learning
4. **Research**: Reliable baseline for performance studies
5. **No External Dependencies**: Easy to install and use

### Container Compatibility

Perfect for containerized environments:

```dockerfile
# Works in minimal containers
FROM alpine:latest
RUN apk add --no-cache cmake gcc g++ make

# Build portable binaries
COPY . /src
WORKDIR /src
RUN cmake -B build -DRAFAELIA_BAREMETAL=ON && \
    cmake --build build --config Release
```

### Cross-Compilation

The portable build mode works well for cross-compilation:

```bash
# Example: Cross-compile for ARM64 from x86-64
cmake -B build -DRAFAELIA_BAREMETAL=ON \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc
cmake --build build --config Release
```

## Testing Across Platforms

### Continuous Integration

The module is tested on multiple platforms through CI:
- Ubuntu 20.04, 22.04, 24.04 (x86-64)
- macOS 12, 13, 14 (Intel and Apple Silicon)
- Windows Server 2019, 2022 (MSVC and MinGW)

### Manual Testing

Test on your platform:

```bash
# Build
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release

# Run comprehensive tests
./build/bin/rafaelia-test

# Run examples
./build/bin/rafaelia-example
./build/bin/rafaelia-integration
```

All tests should pass with output showing:
- Correct architecture detection
- Proper core count detection
- All module tests passing

## Troubleshooting

### Issue: Illegal Instruction Error

**Symptom**: Program crashes with "Illegal instruction" or SIGILL

**Cause**: Binary built with native optimizations running on different CPU

**Solution**: Rebuild with portable mode:
```bash
cmake -B build -DRAFAELIA_BAREMETAL=ON -DRAFAELIA_NATIVE_OPTIMIZE=OFF
cmake --build build --config Release
```

### Issue: Wrong Core Count

**Symptom**: `raf_get_num_cores()` returns 1 instead of actual count

**Cause**: Platform-specific headers not available

**Solution**: Verify your system has POSIX headers (Linux/macOS/BSD) or Windows SDK

### Issue: Missing Architecture Detection

**Symptom**: `raf_detect_arch()` returns `RAF_ARCH_UNKNOWN`

**Cause**: Unsupported architecture or missing compiler defines

**Solution**: The module will still work with safe defaults. For proper support, report the issue with your architecture details.

## Future Improvements

Planned compatibility enhancements:

1. **WebAssembly Support**: Target wasm32/wasm64
2. **More Architectures**: SPARC, S390x, LoongArch
3. **Enhanced Feature Detection**: Better cache topology detection
4. **Runtime Dispatch**: Choose optimal code path at runtime
5. **Accelerator Support**: GPU, FPGA, NPU integration

## Contributing

When adding new features, please ensure:

1. **Portable by default**: Use standard C11 only
2. **Platform guards**: Wrap platform-specific code with `#ifdef`
3. **Fallback paths**: Always provide safe defaults
4. **Test on multiple platforms**: CI or manual testing
5. **Document compatibility**: Update this guide

## Support

For compatibility issues:

1. Check this guide first
2. Review [README.md](README.md) for build instructions
3. Run tests to verify your platform
4. Report issues with platform details (OS, arch, compiler)

## Conclusion

The Rafaelia Baremetal Module prioritizes compatibility without sacrificing performance. By using portable defaults and providing optional native optimizations, it works across diverse environments from academic clusters to production servers, from personal laptops to cloud containers.
