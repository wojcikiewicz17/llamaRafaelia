/**
 * Rafaelia Baremetal - Hardware Detection Implementation
 * 
 * Pure C implementation for hardware detection and CPU register identification.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_hardware.h"

/* Platform-specific headers for core detection */
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #include <unistd.h>
#endif

/* Architecture Detection */
raf_arch_type raf_detect_arch(void) {
    #if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
        return RAF_ARCH_X86_64;
    #elif defined(__i386__) || defined(_M_IX86)
        return RAF_ARCH_X86_32;
    #elif defined(__aarch64__) || defined(_M_ARM64)
        return RAF_ARCH_ARM_64;
    #elif defined(__arm__) || defined(_M_ARM)
        return RAF_ARCH_ARM_32;
    #elif defined(__riscv) && (__riscv_xlen == 64)
        return RAF_ARCH_RISCV_64;
    #elif defined(__riscv) && (__riscv_xlen == 32)
        return RAF_ARCH_RISCV_32;
    #elif defined(__mips__)
        return RAF_ARCH_MIPS;
    #elif defined(__powerpc__) || defined(__ppc__)
        return RAF_ARCH_POWERPC;
    #else
        return RAF_ARCH_UNKNOWN;
    #endif
}

const char* raf_arch_name(raf_arch_type arch) {
    switch (arch) {
        case RAF_ARCH_X86_32: return "x86 32-bit";
        case RAF_ARCH_X86_64: return "x86 64-bit";
        case RAF_ARCH_ARM_32: return "ARM 32-bit";
        case RAF_ARCH_ARM_64: return "ARM 64-bit (AArch64)";
        case RAF_ARCH_RISCV_32: return "RISC-V 32-bit";
        case RAF_ARCH_RISCV_64: return "RISC-V 64-bit";
        case RAF_ARCH_MIPS: return "MIPS";
        case RAF_ARCH_POWERPC: return "PowerPC";
        default: return "Unknown";
    }
}

/* CPUID for x86/x64 */
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)

#if defined(_MSC_VER)
#include <intrin.h>
void raf_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, 
               unsigned int *ecx, unsigned int *edx) {
    int regs[4];
    __cpuid(regs, leaf);
    *eax = regs[0];
    *ebx = regs[1];
    *ecx = regs[2];
    *edx = regs[3];
}
#else
void raf_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, 
               unsigned int *ecx, unsigned int *edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}
#endif

#endif /* x86/x64 */

/* CPU Feature Detection */
void raf_detect_cpu_features(raf_cpu_features *features) {
    /* Initialize all to zero */
    features->arch = raf_detect_arch();
    features->has_sse = 0;
    features->has_sse2 = 0;
    features->has_sse3 = 0;
    features->has_ssse3 = 0;
    features->has_sse41 = 0;
    features->has_sse42 = 0;
    features->has_avx = 0;
    features->has_avx2 = 0;
    features->has_avx512 = 0;
    features->has_fma = 0;
    features->has_neon = 0;
    features->has_sve = 0;
    features->num_cores = raf_get_num_cores();
    features->cache_line_size = raf_get_cache_line_size();
    
    #if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    unsigned int eax, ebx, ecx, edx;
    
    /* Check standard features (leaf 1) */
    raf_cpuid(1, &eax, &ebx, &ecx, &edx);
    
    features->has_sse = (edx >> 25) & 1;
    features->has_sse2 = (edx >> 26) & 1;
    features->has_sse3 = ecx & 1;
    features->has_ssse3 = (ecx >> 9) & 1;
    features->has_sse41 = (ecx >> 19) & 1;
    features->has_sse42 = (ecx >> 20) & 1;
    features->has_avx = (ecx >> 28) & 1;
    features->has_fma = (ecx >> 12) & 1;
    
    /* Check extended features (leaf 7) */
    raf_cpuid(7, &eax, &ebx, &ecx, &edx);
    features->has_avx2 = (ebx >> 5) & 1;
    features->has_avx512 = (ebx >> 16) & 1;
    
    #elif defined(__aarch64__) || defined(__arm__)
    /* ARM NEON detection */
    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
    features->has_neon = 1;
    #endif
    
    /* ARM SVE detection */
    #if defined(__ARM_FEATURE_SVE)
    features->has_sve = 1;
    #endif
    #endif
}

/* Cache Line Size Detection */
unsigned int raf_get_cache_line_size(void) {
    #if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    unsigned int eax, ebx, ecx, edx;
    raf_cpuid(1, &eax, &ebx, &ecx, &edx);
    return ((ebx >> 8) & 0xFF) * 8;
    #elif defined(__aarch64__) || defined(__arm__)
    /* ARM typically uses 64-byte cache lines */
    return 64;
    #else
    /* Default assumption */
    return 64;
    #endif
}

/* Cache Size Detection (simplified) */
unsigned int raf_get_l1_cache_size(void) {
    /* Typical L1 data cache size - would need more complex detection */
    return 32 * 1024; /* 32 KB */
}

unsigned int raf_get_l2_cache_size(void) {
    /* Typical L2 cache size */
    return 256 * 1024; /* 256 KB */
}

unsigned int raf_get_l3_cache_size(void) {
    /* Typical L3 cache size */
    return 8 * 1024 * 1024; /* 8 MB */
}

/* Core Count Detection */
int raf_get_num_cores(void) {
    #if defined(_WIN32) || defined(_WIN64)
    /* Windows - use GetSystemInfo */
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
    #elif defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    /* POSIX systems - use sysconf */
    #ifdef _SC_NPROCESSORS_ONLN
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs > 0) {
        return (int)nprocs;
    }
    #endif
    /* Fallback to _SC_NPROCESSORS_CONF if available */
    #ifdef _SC_NPROCESSORS_CONF
    long nprocs_conf = sysconf(_SC_NPROCESSORS_CONF);
    if (nprocs_conf > 0) {
        return (int)nprocs_conf;
    }
    #endif
    return 1; /* Fallback */
    #else
    /* Unknown platform - default to 1 */
    return 1;
    #endif
}

int raf_get_num_threads(void) {
    /* For now, same as num_cores */
    /* Could detect hyperthreading in the future using CPUID on x86 */
    return raf_get_num_cores();
}
