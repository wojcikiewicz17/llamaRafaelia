/**
 * Rafaelia Baremetal - Hardware Detection and Register Access
 * 
 * Pure C implementation for hardware detection and CPU register identification.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_HARDWARE_H
#define RAFAELIA_HARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* CPU Architecture Detection */
typedef enum {
    RAF_ARCH_UNKNOWN = 0,
    RAF_ARCH_X86_32,
    RAF_ARCH_X86_64,
    RAF_ARCH_ARM_32,
    RAF_ARCH_ARM_64,
    RAF_ARCH_RISCV_32,
    RAF_ARCH_RISCV_64,
    RAF_ARCH_MIPS,
    RAF_ARCH_POWERPC
} raf_arch_type;

/* CPU Features */
typedef struct {
    raf_arch_type arch;
    int has_sse;
    int has_sse2;
    int has_sse3;
    int has_ssse3;
    int has_sse41;
    int has_sse42;
    int has_avx;
    int has_avx2;
    int has_avx512;
    int has_fma;
    int has_neon;
    int has_sve;
    int num_cores;
    unsigned int cache_line_size;
} raf_cpu_features;

/* Hardware Detection Functions */
raf_arch_type raf_detect_arch(void);
void raf_detect_cpu_features(raf_cpu_features *features);
const char* raf_arch_name(raf_arch_type arch);

/* CPU ID and Register Access (x86/x64) */
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
void raf_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, 
               unsigned int *ecx, unsigned int *edx);
#endif

/* Cache Information */
unsigned int raf_get_cache_line_size(void);
unsigned int raf_get_l1_cache_size(void);
unsigned int raf_get_l2_cache_size(void);
unsigned int raf_get_l3_cache_size(void);

/* Core Count Detection */
int raf_get_num_cores(void);
int raf_get_num_threads(void);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_HARDWARE_H */
