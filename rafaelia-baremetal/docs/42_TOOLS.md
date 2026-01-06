# Rafaelia Baremetal - 42 Utility Tools Reference

This document describes all 42 utility tools available in the Rafaelia Baremetal Module.

## Tools 1-5: Memory Operations

### Tool 1: `raf_memcmp_secure`
**Purpose:** Constant-time memory comparison resistant to timing attacks  
**Signature:** `int raf_memcmp_secure(const void *a, const void *b, size_t n)`  
**Use Case:** Comparing passwords, cryptographic keys, or sensitive data

### Tool 2: `raf_memzero_secure`
**Purpose:** Secure memory zeroing that won't be optimized away by compiler  
**Signature:** `void raf_memzero_secure(void *ptr, size_t n)`  
**Use Case:** Clearing sensitive data from memory

### Tool 3: `raf_memcpy_safe`
**Purpose:** Memory copy with overlap detection and bounds checking  
**Signature:** `int raf_memcpy_safe(void *dst, const void *src, size_t n, size_t dst_size)`  
**Use Case:** Safe buffer copying with validation

### Tool 4: `raf_memswap`
**Purpose:** In-place memory swap without temporary buffer  
**Signature:** `void raf_memswap(void *a, void *b, size_t n)`  
**Use Case:** Swapping array elements, buffer swapping

### Tool 5: `raf_memfill_pattern`
**Purpose:** Fill memory with a repeating pattern  
**Signature:** `void raf_memfill_pattern(void *ptr, size_t n, const uint8_t *pattern, size_t pattern_size)`  
**Use Case:** Memory initialization, testing, padding

## Tools 6-10: String Operations

### Tool 6: `raf_strnlen_safe`
**Purpose:** String length calculation with maximum bound  
**Signature:** `size_t raf_strnlen_safe(const char *str, size_t maxlen)`  
**Use Case:** Safe string length checking

### Tool 7: `raf_strcpy_safe`
**Purpose:** String copy with guaranteed null termination  
**Signature:** `size_t raf_strcpy_safe(char *dst, const char *src, size_t dst_size)`  
**Use Case:** Buffer-safe string copying

### Tool 8: `raf_strcat_safe`
**Purpose:** String concatenation with bounds checking  
**Signature:** `size_t raf_strcat_safe(char *dst, const char *src, size_t dst_size)`  
**Use Case:** Safe string concatenation

### Tool 9: `raf_strcasecmp`
**Purpose:** Case-insensitive string comparison  
**Signature:** `int raf_strcasecmp(const char *s1, const char *s2)`  
**Use Case:** Command parsing, configuration matching

### Tool 10: `raf_strtok_safe`
**Purpose:** Thread-safe string tokenization  
**Signature:** `char* raf_strtok_safe(char *str, const char *delim, char **saveptr)`  
**Use Case:** Parsing CSV, command-line arguments

## Tools 11-15: Math Operations

### Tool 11: `raf_isqrt`
**Purpose:** Fast integer square root  
**Signature:** `uint32_t raf_isqrt(uint32_t n)`  
**Use Case:** Distance calculations, graphics

### Tool 12: `raf_ipow`
**Purpose:** Integer exponentiation  
**Signature:** `uint64_t raf_ipow(uint64_t base, uint32_t exp)`  
**Use Case:** Power calculations, scaling

### Tool 13: `raf_gcd`
**Purpose:** Greatest common divisor (Euclidean algorithm)  
**Signature:** `uint64_t raf_gcd(uint64_t a, uint64_t b)`  
**Use Case:** Fraction reduction, synchronization

### Tool 14: `raf_lcm`
**Purpose:** Least common multiple  
**Signature:** `uint64_t raf_lcm(uint64_t a, uint64_t b)`  
**Use Case:** Timing calculations, scheduling

### Tool 15: `raf_modpow`
**Purpose:** Modular exponentiation for cryptography  
**Signature:** `uint64_t raf_modpow(uint64_t base, uint64_t exp, uint64_t mod)`  
**Use Case:** RSA, Diffie-Hellman, primality testing

## Tools 16-20: Bit Operations

### Tool 16: `raf_popcount`
**Purpose:** Count set bits (population count)  
**Signature:** `uint32_t raf_popcount(uint64_t x)`  
**Use Case:** Hamming distance, bitboards

### Tool 17: `raf_ffs`
**Purpose:** Find first set bit  
**Signature:** `uint32_t raf_ffs(uint64_t x)`  
**Use Case:** Bit scanning, sparse data

### Tool 18: `raf_fls`
**Purpose:** Find last set bit  
**Signature:** `uint32_t raf_fls(uint64_t x)`  
**Use Case:** Log2 approximation, MSB finding

### Tool 19: `raf_rotl`
**Purpose:** Rotate bits left  
**Signature:** `uint64_t raf_rotl(uint64_t x, uint32_t n)`  
**Use Case:** Cryptographic operations, hashing

### Tool 20: `raf_rotr`
**Purpose:** Rotate bits right  
**Signature:** `uint64_t raf_rotr(uint64_t x, uint32_t n)`  
**Use Case:** Cryptographic operations, hashing

## Tools 21-25: Checksum and Hashing

### Tool 21: `raf_fletcher16`
**Purpose:** Fletcher-16 checksum  
**Signature:** `uint16_t raf_fletcher16(const uint8_t *data, size_t len)`  
**Use Case:** Network protocols, error detection

### Tool 22: `raf_fletcher32`
**Purpose:** Fletcher-32 checksum  
**Signature:** `uint32_t raf_fletcher32(const uint16_t *data, size_t len)`  
**Use Case:** TCP checksum alternative

### Tool 23: `raf_adler32`
**Purpose:** Adler-32 checksum (used in zlib)  
**Signature:** `uint32_t raf_adler32(const uint8_t *data, size_t len)`  
**Use Case:** Compression, data integrity

### Tool 24: `raf_murmur3_32`
**Purpose:** MurmurHash3 32-bit hash  
**Signature:** `uint32_t raf_murmur3_32(const void *key, size_t len, uint32_t seed)`  
**Use Case:** Hash tables, bloom filters

### Tool 25: `raf_hash64`
**Purpose:** FNV-1a 64-bit hash  
**Signature:** `uint64_t raf_hash64(const uint8_t *data, size_t len)`  
**Use Case:** Fast hashing, checksums

## Tools 26-30: Encoding/Decoding

### Tool 26: `raf_base64_encode`
**Purpose:** Base64 encoding  
**Signature:** `size_t raf_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len)`  
**Use Case:** Binary data transmission, email encoding

### Tool 27: `raf_base64_decode`
**Purpose:** Base64 decoding  
**Signature:** `size_t raf_base64_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_len)`  
**Use Case:** Decoding base64-encoded data

### Tool 28: `raf_hex_encode`
**Purpose:** Hexadecimal encoding  
**Signature:** `size_t raf_hex_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len)`  
**Use Case:** Debugging, hash display

### Tool 29: `raf_hex_decode`
**Purpose:** Hexadecimal decoding  
**Signature:** `size_t raf_hex_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_len)`  
**Use Case:** Parsing hex strings

### Tool 30: `raf_url_encode`
**Purpose:** URL/percent encoding  
**Signature:** `size_t raf_url_encode(const char *src, char *dst, size_t dst_len)`  
**Use Case:** HTTP requests, query parameters

## Tools 31-35: Random Number Generation

### Tool 31: `raf_prng_init`
**Purpose:** Initialize PRNG with seed  
**Signature:** `void raf_prng_init(raf_prng *prng, uint64_t seed)`  
**Use Case:** Setting up random number generator

### Tool 32: `raf_prng_next`
**Purpose:** Generate next random number  
**Signature:** `uint64_t raf_prng_next(raf_prng *prng)`  
**Use Case:** Random number generation (xoshiro256**)

### Tool 33: `raf_prng_range`
**Purpose:** Generate random number in range  
**Signature:** `uint64_t raf_prng_range(raf_prng *prng, uint64_t max)`  
**Use Case:** Random selection, dice rolling

### Tool 34: `raf_prng_float`
**Purpose:** Generate random float [0, 1)  
**Signature:** `float raf_prng_float(raf_prng *prng)`  
**Use Case:** Monte Carlo simulations, noise

### Tool 35: `raf_prng_bytes`
**Purpose:** Fill buffer with random bytes  
**Signature:** `void raf_prng_bytes(raf_prng *prng, uint8_t *buf, size_t len)`  
**Use Case:** Key generation, initialization vectors

## Tools 36-40: Sorting and Searching

### Tool 36: `raf_qsort_int`
**Purpose:** Quicksort for integer arrays  
**Signature:** `void raf_qsort_int(int32_t *array, size_t len)`  
**Use Case:** Fast sorting of integers

### Tool 37: `raf_bsearch_int`
**Purpose:** Binary search in sorted array  
**Signature:** `int raf_bsearch_int(const int32_t *array, size_t len, int32_t key)`  
**Use Case:** Fast lookups in sorted data

### Tool 38: `raf_isort_int`
**Purpose:** Insertion sort (stable, good for small arrays)  
**Signature:** `void raf_isort_int(int32_t *array, size_t len)`  
**Use Case:** Sorting small arrays, nearly-sorted data

### Tool 39: `raf_min_int`
**Purpose:** Find minimum value in array  
**Signature:** `int32_t raf_min_int(const int32_t *array, size_t len)`  
**Use Case:** Statistics, bounds finding

### Tool 40: `raf_max_int`
**Purpose:** Find maximum value in array  
**Signature:** `int32_t raf_max_int(const int32_t *array, size_t len)`  
**Use Case:** Statistics, bounds finding

## Tools 41-42: Time and Profiling

### Tool 41: `raf_timestamp`
**Purpose:** High-resolution timestamp  
**Signature:** `uint64_t raf_timestamp(void)`  
**Use Case:** Performance measurement, profiling

### Tool 42: `raf_delay_us`
**Purpose:** Microsecond delay  
**Signature:** `void raf_delay_us(uint32_t microseconds)`  
**Use Case:** Timing, hardware interfacing

---

## Summary

These 42 tools provide a comprehensive baremetal toolkit covering:
- **Memory safety** (Tools 1-5)
- **String handling** (Tools 6-10)
- **Mathematical operations** (Tools 11-15)
- **Bit manipulation** (Tools 16-20)
- **Checksums and hashing** (Tools 21-25)
- **Data encoding** (Tools 26-30)
- **Random generation** (Tools 31-35)
- **Data structures** (Tools 36-40)
- **Performance** (Tools 41-42)

All tools are implemented in pure C with no external dependencies, making them suitable for baremetal environments and embedded systems.
