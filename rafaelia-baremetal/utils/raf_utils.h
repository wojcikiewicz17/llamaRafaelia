/**
 * Rafaelia Baremetal - Utility Tools Collection
 * 
 * Collection of specialized utility functions for baremetal operations.
 * Part of the 42 tools suite.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_UTILS_H
#define RAFAELIA_UTILS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Tool 1-5: Memory Operations ===== */

/* Memory comparison with timing-attack resistance */
int raf_memcmp_secure(const void *a, const void *b, size_t n);

/* Memory zeroing that won't be optimized away */
void raf_memzero_secure(void *ptr, size_t n);

/* Memory copy with overlap detection */
int raf_memcpy_safe(void *dst, const void *src, size_t n, size_t dst_size);

/* Memory swap without temporary buffer */
void raf_memswap(void *a, void *b, size_t n);

/* Memory pattern fill */
void raf_memfill_pattern(void *ptr, size_t n, const uint8_t *pattern, size_t pattern_size);

/* ===== Tool 6-10: String Operations ===== */

/* String length with maximum bound */
size_t raf_strnlen_safe(const char *str, size_t maxlen);

/* String copy with guaranteed null termination */
size_t raf_strcpy_safe(char *dst, const char *src, size_t dst_size);

/* String concatenation with bounds checking */
size_t raf_strcat_safe(char *dst, const char *src, size_t dst_size);

/* Case-insensitive string comparison */
int raf_strcasecmp(const char *s1, const char *s2);

/* String tokenization (thread-safe) */
char* raf_strtok_safe(char *str, const char *delim, char **saveptr);

/* ===== Tool 11-15: Math Operations ===== */

/* Fast integer square root */
uint32_t raf_isqrt(uint32_t n);

/* Integer power function */
uint64_t raf_ipow(uint64_t base, uint32_t exp);

/* Greatest common divisor */
uint64_t raf_gcd(uint64_t a, uint64_t b);

/* Least common multiple */
uint64_t raf_lcm(uint64_t a, uint64_t b);

/* Fast modular exponentiation */
uint64_t raf_modpow(uint64_t base, uint64_t exp, uint64_t mod);

/* ===== Tool 16-20: Bit Operations ===== */

/* Population count (number of set bits) */
uint32_t raf_popcount(uint64_t x);

/* Find first set bit (1-indexed, 0 if none) */
uint32_t raf_ffs(uint64_t x);

/* Find last set bit (1-indexed, 0 if none) */
uint32_t raf_fls(uint64_t x);

/* Rotate left */
uint64_t raf_rotl(uint64_t x, uint32_t n);

/* Rotate right */
uint64_t raf_rotr(uint64_t x, uint32_t n);

/* ===== Tool 21-25: Checksum and Hashing ===== */

/* Fletcher-16 checksum */
uint16_t raf_fletcher16(const uint8_t *data, size_t len);

/* Fletcher-32 checksum */
uint32_t raf_fletcher32(const uint16_t *data, size_t len);

/* Adler-32 checksum */
uint32_t raf_adler32(const uint8_t *data, size_t len);

/* MurmurHash3 (32-bit) */
uint32_t raf_murmur3_32(const void *key, size_t len, uint32_t seed);

/* Simple hash function for strings */
uint64_t raf_hash64(const uint8_t *data, size_t len);

/* ===== Tool 26-30: Encoding/Decoding ===== */

/* Base64 encode */
size_t raf_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len);

/* Base64 decode */
size_t raf_base64_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_len);

/* Hexadecimal encode */
size_t raf_hex_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len);

/* Hexadecimal decode */
size_t raf_hex_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_len);

/* URL encoding */
size_t raf_url_encode(const char *src, char *dst, size_t dst_len);

/* ===== Tool 31-35: Random Number Generation ===== */

/* Simple PRNG state */
typedef struct {
    uint64_t state[4];
} raf_prng;

/* Initialize PRNG with seed */
void raf_prng_init(raf_prng *prng, uint64_t seed);

/* Generate random uint64_t */
uint64_t raf_prng_next(raf_prng *prng);

/* Generate random in range [0, max) */
uint64_t raf_prng_range(raf_prng *prng, uint64_t max);

/* Generate random float in [0, 1) */
float raf_prng_float(raf_prng *prng);

/* Fill buffer with random bytes */
void raf_prng_bytes(raf_prng *prng, uint8_t *buf, size_t len);

/* ===== Tool 36-40: Sorting and Searching ===== */

/* Quicksort for integers */
void raf_qsort_int(int32_t *array, size_t len);

/* Binary search in sorted array */
int raf_bsearch_int(const int32_t *array, size_t len, int32_t key);

/* Insertion sort (stable, good for small arrays) */
void raf_isort_int(int32_t *array, size_t len);

/* Find minimum value in array */
int32_t raf_min_int(const int32_t *array, size_t len);

/* Find maximum value in array */
int32_t raf_max_int(const int32_t *array, size_t len);

/* ===== Tool 41-42: Time and Profiling ===== */

/* High-resolution timestamp (cycles or nanoseconds) */
uint64_t raf_timestamp(void);

/* Microsecond delay */
void raf_delay_us(uint32_t microseconds);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_UTILS_H */
