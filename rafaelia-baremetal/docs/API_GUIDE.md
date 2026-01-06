# Rafaelia Baremetal - Complete API Usage Guide

## Overview

This guide provides comprehensive examples and usage patterns for all modules in the Rafaelia Baremetal framework. Each section demonstrates practical applications of the APIs with complete code examples.

## Table of Contents

1. [Hardware Detection Module](#hardware-detection-module)
2. [Core Linear Algebra Module](#core-linear-algebra-module)
3. [BITRAF - Bit Operations Module](#bitraf---bit-operations-module)
4. [BITSTACK - Non-linear Logic Module](#bitstack---non-linear-logic-module)
5. [ZIPRAF - Archiving Module](#zipraf---archiving-module)
6. [RAFSTORE - Storage Management Module](#rafstore---storage-management-module)
7. [TOROID - Topology Module](#toroid---topology-module)
8. [Utils - 42 Tools Collection](#utils---42-tools-collection)

---

## Hardware Detection Module

### Basic CPU Feature Detection

```c
#include "hardware/raf_hardware.h"

void detect_hardware_capabilities(void) {
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    
    printf("Architecture: %s\n", raf_arch_name(features.arch));
    printf("Bits: %d\n", features.is_64bit ? 64 : 32);
    printf("CPU Cores: %d\n", features.num_cores);
    printf("Cache Line: %u bytes\n", features.cache_line_size);
    printf("Has SSE: %s\n", features.has_sse ? "Yes" : "No");
    printf("Has AVX: %s\n", features.has_avx ? "Yes" : "No");
    printf("Has AVX2: %s\n", features.has_avx2 ? "Yes" : "No");
    printf("Has NEON: %s\n", features.has_neon ? "Yes" : "No");
}
```

### Runtime Optimization Selection

```c
void process_data_optimized(float *data, size_t len) {
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    
    if (features.has_avx2) {
        // Use AVX2 optimized path
        process_with_avx2(data, len);
    } else if (features.has_sse) {
        // Fall back to SSE
        process_with_sse(data, len);
    } else {
        // Generic path
        process_generic(data, len);
    }
}
```

---

## Core Linear Algebra Module

### Matrix Operations

```c
#include "core/raf_core.h"

void matrix_computation_example(void) {
    // Create matrices
    raf_matrix *a = raf_matrix_create(3, 3);
    raf_matrix *b = raf_matrix_create(3, 3);
    raf_matrix *result = raf_matrix_create(3, 3);
    
    // Initialize with identity
    raf_matrix_identity(a);
    raf_matrix_identity(b);
    
    // Set specific values
    raf_matrix_set(a, 0, 0, 2.0f);
    raf_matrix_set(a, 1, 1, 3.0f);
    raf_matrix_set(a, 2, 2, 4.0f);
    
    // Matrix multiplication
    raf_matrix_mul(a, b, result);
    
    // Scale result
    raf_matrix *scaled = raf_matrix_create(3, 3);
    raf_matrix_scale(result, 2.0f, scaled);
    
    // Element-wise operations
    raf_matrix_elem_mul(a, b, result);
    
    // Transpose
    raf_matrix *transposed = raf_matrix_create(3, 3);
    raf_matrix_transpose(a, transposed);
    
    // Cleanup
    raf_matrix_destroy(a);
    raf_matrix_destroy(b);
    raf_matrix_destroy(result);
    raf_matrix_destroy(scaled);
    raf_matrix_destroy(transposed);
}
```

### Vector Operations

```c
void vector_computation_example(void) {
    // Create vectors
    raf_vector *v1 = raf_vector_create(3);
    raf_vector *v2 = raf_vector_create(3);
    raf_vector *result = raf_vector_create(3);
    
    // Set values
    v1->data[0] = 1.0f; v1->data[1] = 2.0f; v1->data[2] = 3.0f;
    v2->data[0] = 4.0f; v2->data[1] = 5.0f; v2->data[2] = 6.0f;
    
    // Vector operations
    raf_vector_add(v1, v2, result);  // [5, 7, 9]
    raf_scalar dot = raf_vector_dot(v1, v2);  // 32.0
    raf_scalar norm = raf_vector_norm(v1);    // sqrt(14)
    
    // Scale vector
    raf_vector_scale(v1, 2.0f, result);  // [2, 4, 6]
    
    // Cleanup
    raf_vector_destroy(v1);
    raf_vector_destroy(v2);
    raf_vector_destroy(result);
}
```

---

## BITRAF - Bit Operations Module

### Bitstream Operations

```c
#include "bitraf/raf_bitraf.h"

void bitstream_example(void) {
    // Create bitstream
    raf_bitstream *bs = raf_bitstream_create(256);
    
    // Write individual bits
    raf_bitstream_write_bit(bs, 1);
    raf_bitstream_write_bit(bs, 0);
    raf_bitstream_write_bit(bs, 1);
    
    // Write multi-bit values
    raf_bitstream_write_bits(bs, 0xA, 4);  // Write 1010
    raf_bitstream_write_bits(bs, 0x1F, 5); // Write 11111
    
    // Write full bytes
    raf_bitstream_write_byte(bs, 0xFF);
    
    // Reset for reading
    raf_bitstream_reset(bs);
    
    // Read back
    uint8_t bit;
    raf_bitstream_read_bit(bs, &bit);
    
    uint32_t value;
    raf_bitstream_read_bits(bs, &value, 4);
    
    uint8_t byte;
    raf_bitstream_read_byte(bs, &byte);
    
    raf_bitstream_destroy(bs);
}
```

### Bit Manipulation Utilities

```c
void bit_utilities_example(void) {
    uint32_t value = 0x12345678;
    
    // Count set bits
    uint8_t count = raf_count_bits(value);
    
    // Count leading/trailing zeros
    uint8_t leading = raf_count_leading_zeros(value);
    uint8_t trailing = raf_count_trailing_zeros(value);
    
    // Reverse bits
    uint32_t reversed = raf_reverse_bits(value, 32);
    
    printf("Value: 0x%08X\n", value);
    printf("Bit count: %u\n", count);
    printf("Leading zeros: %u\n", leading);
    printf("Trailing zeros: %u\n", trailing);
    printf("Reversed: 0x%08X\n", reversed);
}
```

---

## BITSTACK - Non-linear Logic Module

### Basic Bitstack Operations

```c
#include "bitstack/raf_bitstack.h"

void bitstack_basic_example(void) {
    // Create bitstack with 4 layers, 64 bits per layer
    raf_bitstack_state *state = raf_bitstack_create(4, 64);
    
    // Set individual bits
    raf_bitstack_set_bit(state, 0, 10, 1);
    raf_bitstack_set_bit(state, 1, 20, 1);
    raf_bitstack_set_bit(state, 2, 30, 1);
    
    // Get bit value
    int bit_value = raf_bitstack_get_bit(state, 0, 10);
    
    // Count bits in a layer
    uint32_t count = raf_bitstack_count_bits(state, 0);
    
    printf("Bit at (0,10): %d\n", bit_value);
    printf("Bits set in layer 0: %u\n", count);
    
    raf_bitstack_destroy(state);
}
```

### Single-Flip Operations and Cascades

```c
void bitstack_flip_example(void) {
    raf_bitstack_state *state = raf_bitstack_create(4, 64);
    
    // Single flip
    raf_bitstack_flip_single(state, 0, 10);
    
    // Evaluate flip effect without modifying
    double effect = raf_bitstack_evaluate_flip(state, 1, 20);
    printf("Flip effect: %.4f\n", effect);
    
    // Cascade flip with non-linear propagation
    uint32_t flipped = raf_bitstack_flip_cascade(state, 2, 30, 5);
    printf("Cascade flipped %u bits\n", flipped);
    
    raf_bitstack_destroy(state);
}
```

### Opportunity Detection

```c
void bitstack_opportunity_example(void) {
    raf_bitstack_state *state = raf_bitstack_create(4, 64);
    
    // Initialize with some pattern
    for (uint32_t i = 0; i < 16; i++) {
        raf_bitstack_set_bit(state, 0, i, 1);
    }
    
    // Create result structure
    raf_calc_result *result = raf_calc_result_create(20);
    
    // Scan for optimal opportunities
    raf_bitstack_scan_optimal(state, result, 8);
    
    printf("Found %zu opportunities\n", result->num_opportunities);
    if (result->num_opportunities > 0) {
        printf("Best: position %u, confidence %.2f\n",
               result->opportunities[0].bit_positions[0],
               result->opportunities[0].confidence);
    }
    
    raf_calc_result_destroy(result);
    raf_bitstack_destroy(state);
}
```

### Deterministic Calculations

```c
void bitstack_calculations_example(void) {
    raf_bitstack_state *state1 = raf_bitstack_create(4, 64);
    raf_bitstack_state *state2 = raf_bitstack_create(4, 64);
    
    // Initialize states
    for (uint32_t i = 0; i < 32; i++) {
        raf_bitstack_set_bit(state1, 0, i, 1);
        raf_bitstack_set_bit(state2, 0, i + 10, 1);
    }
    
    // Calculate hash for state tracking
    uint64_t hash1 = raf_bitstack_calc_hash(state1);
    uint64_t hash2 = raf_bitstack_calc_hash(state2);
    
    // Calculate Hamming distance
    uint32_t distance = raf_bitstack_calc_distance(state1, state2);
    
    // Calculate entropy
    double entropy1 = raf_bitstack_calc_entropy(state1);
    
    // Calculate correlation between layers
    double corr = raf_bitstack_calc_correlation(state1, 0, 1);
    
    printf("Hash1: 0x%016llX\n", (unsigned long long)hash1);
    printf("Hash2: 0x%016llX\n", (unsigned long long)hash2);
    printf("Distance: %u bits\n", distance);
    printf("Entropy: %.4f\n", entropy1);
    printf("Correlation: %.4f\n", corr);
    
    raf_bitstack_destroy(state1);
    raf_bitstack_destroy(state2);
}
```

---

## ZIPRAF - Archiving Module

### Archive Creation and Management

```c
#include "zipraf/raf_zipraf.h"

void archive_example(void) {
    // Create archive
    raf_zipraf_archive *archive = raf_zipraf_create();
    
    // Add data entries
    uint8_t data1[] = "Hello, World!";
    uint8_t data2[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    raf_zipraf_add_data(archive, "message.txt", data1, sizeof(data1) - 1);
    raf_zipraf_add_data(archive, "binary.dat", data2, sizeof(data2));
    
    // List entries
    int count = raf_zipraf_list_entries(archive);
    printf("Archive contains %d entries\n", count);
    
    // Extract data
    uint8_t *extracted;
    size_t extracted_len;
    if (raf_zipraf_extract_data(archive, "message.txt", &extracted, &extracted_len) == 0) {
        printf("Extracted: %s (%zu bytes)\n", extracted, extracted_len);
        free(extracted);
    }
    
    raf_zipraf_destroy(archive);
}
```

### CRC32 Checksums

```c
void crc32_example(void) {
    const char *data = "Test data for checksum";
    uint32_t crc = raf_zipraf_crc32((uint8_t*)data, strlen(data));
    
    printf("Data: %s\n", data);
    printf("CRC32: 0x%08X\n", crc);
    
    // Verify integrity
    uint32_t crc_check = raf_zipraf_crc32((uint8_t*)data, strlen(data));
    if (crc == crc_check) {
        printf("Data integrity: VERIFIED\n");
    }
}
```

---

## RAFSTORE - Storage Management Module

### Memory Pool

```c
#include "rafstore/raf_rafstore.h"

void memory_pool_example(void) {
    // Create memory pool
    raf_memory_pool *pool = raf_pool_create(4096);
    
    // Allocate multiple objects
    void *obj1 = raf_pool_alloc(pool, 64);
    void *obj2 = raf_pool_alloc(pool, 128);
    void *obj3 = raf_pool_alloc(pool, 256);
    
    // Use allocated memory
    memset(obj1, 0, 64);
    memset(obj2, 0, 128);
    memset(obj3, 0, 256);
    
    // Pool cleanup frees all allocations
    raf_pool_destroy(pool);
}
```

### Key-Value Store

```c
void kv_store_example(void) {
    // Create KV store
    raf_kv_store *store = raf_kv_create(32);
    
    // Store different types of data
    int value1 = 42;
    raf_kv_set(store, "answer", &value1, sizeof(value1));
    
    const char *value2 = "Hello, Rafaelia!";
    raf_kv_set(store, "message", value2, strlen(value2) + 1);
    
    // Retrieve data
    void *retrieved;
    size_t size;
    if (raf_kv_get(store, "answer", &retrieved, &size) == 0) {
        printf("Answer: %d\n", *(int*)retrieved);
    }
    
    if (raf_kv_get(store, "message", &retrieved, &size) == 0) {
        printf("Message: %s\n", (char*)retrieved);
    }
    
    // Delete entry
    raf_kv_delete(store, "answer");
    
    raf_kv_destroy(store);
}
```

### Ring Buffer

```c
void ring_buffer_example(void) {
    // Create ring buffer
    raf_ring_buffer *ring = raf_ring_create(256);
    
    // Push data
    for (int i = 0; i < 100; i++) {
        raf_ring_push(ring, (uint8_t)i);
    }
    
    // Check state
    printf("Available: %zu\n", raf_ring_available(ring));
    printf("Empty: %s\n", raf_ring_is_empty(ring) ? "Yes" : "No");
    printf("Full: %s\n", raf_ring_is_full(ring) ? "Yes" : "No");
    
    // Pop data
    while (!raf_ring_is_empty(ring)) {
        uint8_t value;
        if (raf_ring_pop(ring, &value) == 0) {
            // Process value
        }
    }
    
    raf_ring_destroy(ring);
}
```

---

## TOROID - Topology Module

### 2D Toroidal Grid

```c
#include "toroid/raf_toroid.h"

void toroid_2d_example(void) {
    // Create 10x10 toroidal grid
    raf_toroid_2d *grid = raf_toroid_2d_create(10, 10);
    
    // Set values (with automatic wrapping)
    raf_toroid_2d_set(grid, 0, 0, 1.0f);
    raf_toroid_2d_set(grid, -1, -1, 2.0f);  // Wraps to (9, 9)
    
    // Get values
    float val1 = raf_toroid_2d_get(grid, 0, 0);
    float val2 = raf_toroid_2d_get(grid, 9, 9);
    
    // Get neighbors (8-connected)
    float neighbors[8];
    uint32_t count;
    raf_toroid_2d_get_neighbors(grid, 5, 5, neighbors, &count);
    
    // Calculate toroidal distance
    float dist = raf_toroid_2d_distance(grid, 0, 0, 9, 9);
    printf("Toroidal distance: %.2f\n", dist);
    
    raf_toroid_2d_destroy(grid);
}
```

### Heat Diffusion Simulation

```c
void heat_diffusion_example(void) {
    raf_toroid_2d *grid = raf_toroid_2d_create(20, 20);
    raf_toroid_2d *temp = raf_toroid_2d_create(20, 20);
    
    // Initialize with hot spot in center
    raf_toroid_2d_set(grid, 10, 10, 100.0f);
    
    // Simulate diffusion for 100 steps
    for (int step = 0; step < 100; step++) {
        raf_toroid_2d_diffuse(grid, temp, 0.25f);
        // Swap buffers
        raf_toroid_2d *swap = grid;
        grid = temp;
        temp = swap;
    }
    
    // Check final state
    float center = raf_toroid_2d_get(grid, 10, 10);
    printf("Center temperature after diffusion: %.2f\n", center);
    
    raf_toroid_2d_destroy(grid);
    raf_toroid_2d_destroy(temp);
}
```

### Convolution Operations

```c
void convolution_example(void) {
    raf_toroid_2d *grid = raf_toroid_2d_create(20, 20);
    raf_toroid_2d *result = raf_toroid_2d_create(20, 20);
    
    // Initialize input
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 20; x++) {
            raf_toroid_2d_set(grid, x, y, (float)(x + y));
        }
    }
    
    // Define 3x3 kernel (e.g., Gaussian blur)
    float kernel[9] = {
        1/16.0f, 2/16.0f, 1/16.0f,
        2/16.0f, 4/16.0f, 2/16.0f,
        1/16.0f, 2/16.0f, 1/16.0f
    };
    
    // Apply convolution
    raf_toroid_2d_convolve(grid, result, kernel, 3, 3);
    
    raf_toroid_2d_destroy(grid);
    raf_toroid_2d_destroy(result);
}
```

---

## Utils - 42 Tools Collection

### Memory Operations (Tools 1-5)

```c
#include "utils/raf_utils.h"

void memory_tools_example(void) {
    // Secure comparison (timing-attack resistant)
    uint8_t key1[32] = "secret_key_data_xxxxxxxxxx";
    uint8_t key2[32] = "secret_key_data_yyyyyyyyyy";
    int equal = raf_memcmp_secure(key1, key2, 32);
    
    // Secure zeroing (won't be optimized away)
    raf_memzero_secure(key1, sizeof(key1));
    raf_memzero_secure(key2, sizeof(key2));
    
    // Safe copy with bounds checking
    char src[20] = "Hello";
    char dst[10];
    int result = raf_memcpy_safe(dst, src, strlen(src) + 1, sizeof(dst));
    
    // Memory swap without temporary
    int a = 42, b = 84;
    raf_memswap(&a, &b, sizeof(int));
    
    // Pattern fill
    uint8_t buffer[100];
    uint8_t pattern[] = {0xAA, 0xBB, 0xCC, 0xDD};
    raf_memfill_pattern(buffer, sizeof(buffer), pattern, sizeof(pattern));
}
```

### String Operations (Tools 6-10)

```c
void string_tools_example(void) {
    // Safe string length
    const char *str = "Hello, World!";
    size_t len = raf_strnlen_safe(str, 100);
    
    // Safe string copy
    char dest[20];
    raf_strcpy_safe(dest, str, sizeof(dest));
    
    // Safe concatenation
    raf_strcat_safe(dest, " More", sizeof(dest));
    
    // Case-insensitive comparison
    int cmp = raf_strcasecmp("Hello", "HELLO");  // Returns 0
    
    // Thread-safe tokenization
    char input[] = "one,two,three";
    char *saveptr;
    char *token = raf_strtok_safe(input, ",", &saveptr);
    while (token) {
        printf("Token: %s\n", token);
        token = raf_strtok_safe(NULL, ",", &saveptr);
    }
}
```

### Math Operations (Tools 11-15)

```c
void math_tools_example(void) {
    // Integer square root
    uint32_t sqrt_val = raf_isqrt(144);  // Returns 12
    
    // Integer power
    uint64_t power = raf_ipow(2, 10);  // Returns 1024
    
    // Greatest common divisor
    uint32_t gcd = raf_gcd(48, 18);  // Returns 6
    
    // Least common multiple
    uint32_t lcm = raf_lcm(12, 18);  // Returns 36
    
    // Modular exponentiation
    uint64_t mod_exp = raf_modpow(3, 5, 7);  // (3^5) mod 7 = 5
    
    printf("sqrt(144) = %u\n", sqrt_val);
    printf("2^10 = %llu\n", (unsigned long long)power);
    printf("gcd(48, 18) = %u\n", gcd);
    printf("lcm(12, 18) = %u\n", lcm);
    printf("3^5 mod 7 = %llu\n", (unsigned long long)mod_exp);
}
```

### Bit Operations (Tools 16-20)

```c
void bit_tools_example(void) {
    uint64_t value = 0x123456789ABCDEF0ULL;
    
    // Population count
    uint32_t popcount = raf_popcount(value);
    
    // Find first set bit
    int first = raf_ffs(value);
    
    // Find last set bit
    int last = raf_fls(value);
    
    // Rotate left
    uint64_t rotl = raf_rotl(value, 8);
    
    // Rotate right
    uint64_t rotr = raf_rotr(value, 8);
    
    printf("Popcount: %u\n", popcount);
    printf("First set: %d\n", first);
    printf("Last set: %d\n", last);
    printf("Rotate left: 0x%016llX\n", (unsigned long long)rotl);
    printf("Rotate right: 0x%016llX\n", (unsigned long long)rotr);
}
```

### Checksums and Hashing (Tools 21-25)

```c
void checksum_tools_example(void) {
    const char *data = "Test data for checksums";
    
    // Fletcher-16 checksum
    uint16_t fletcher16 = raf_fletcher16((uint8_t*)data, strlen(data));
    
    // Adler-32 checksum
    uint32_t adler32 = raf_adler32((uint8_t*)data, strlen(data));
    
    // MurmurHash3 32-bit
    uint32_t murmur = raf_murmur3_32(data, strlen(data), 0);
    
    // 64-bit hash
    uint64_t hash64 = raf_hash64((uint8_t*)data, strlen(data));
    
    printf("Fletcher16: 0x%04X\n", fletcher16);
    printf("Adler32: 0x%08X\n", adler32);
    printf("MurmurHash3: 0x%08X\n", murmur);
    printf("Hash64: 0x%016llX\n", (unsigned long long)hash64);
}
```

### Encoding/Decoding (Tools 26-30)

```c
void encoding_tools_example(void) {
    // Base64 encoding
    uint8_t binary[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    char base64[100];
    size_t b64_len = raf_base64_encode(binary, sizeof(binary), base64, sizeof(base64));
    printf("Base64: %s\n", base64);
    
    // Hexadecimal encoding
    char hex[100];
    size_t hex_len = raf_hex_encode(binary, sizeof(binary), hex, sizeof(hex));
    printf("Hex: %s\n", hex);
    
    // URL encoding
    const char *url = "Hello World!";
    char encoded[100];
    raf_url_encode(url, encoded, sizeof(encoded));
    printf("URL Encoded: %s\n", encoded);
}
```

### Random Number Generation (Tools 31-35)

```c
void random_tools_example(void) {
    // Initialize PRNG
    raf_prng prng;
    raf_prng_init(&prng, 12345);
    
    // Generate random uint64_t
    uint64_t rand1 = raf_prng_next(&prng);
    
    // Generate in range [0, 100)
    uint64_t rand2 = raf_prng_range(&prng, 100);
    
    // Generate float in [0, 1)
    float rand3 = raf_prng_float(&prng);
    
    // Fill buffer with random bytes
    uint8_t buffer[32];
    raf_prng_bytes(&prng, buffer, sizeof(buffer));
    
    printf("Random uint64: %llu\n", (unsigned long long)rand1);
    printf("Random [0,100): %llu\n", (unsigned long long)rand2);
    printf("Random float: %.6f\n", rand3);
}
```

### Sorting and Searching (Tools 36-40)

```c
void sorting_tools_example(void) {
    int array[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    size_t len = sizeof(array) / sizeof(array[0]);
    
    // Quicksort
    raf_quicksort_int(array, len);
    
    // Binary search
    int key = 7;
    int index = raf_binarysearch_int(array, len, key);
    
    // Find minimum
    int min_idx = raf_find_min_int(array, len);
    
    // Find maximum
    int max_idx = raf_find_max_int(array, len);
    
    printf("Sorted array: ");
    for (size_t i = 0; i < len; i++) printf("%d ", array[i]);
    printf("\n");
    printf("Index of 7: %d\n", index);
    printf("Min value: %d at index %d\n", array[min_idx], min_idx);
    printf("Max value: %d at index %d\n", array[max_idx], max_idx);
}
```

### Time and Profiling (Tools 41-42)

```c
void time_tools_example(void) {
    // Get high-resolution timestamp
    uint64_t start = raf_timestamp();
    
    // Do some work
    for (volatile int i = 0; i < 1000000; i++);
    
    // Measure elapsed time
    uint64_t end = raf_timestamp();
    uint64_t elapsed = end - start;
    
    printf("Elapsed time: %llu time units\n", (unsigned long long)elapsed);
    
    // Precise delay
    raf_delay_us(1000);  // Delay 1000 microseconds (1 ms)
}
```

---

## Complete Integration Example

Here's a complete example that demonstrates multiple modules working together:

```c
#include "rafaelia_baremetal.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // 1. Detect hardware
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    printf("Running on %s with %d cores\n", 
           raf_arch_name(features.arch), features.num_cores);
    
    // 2. Create and manipulate matrices
    raf_matrix *mat = raf_matrix_create(3, 3);
    raf_matrix_identity(mat);
    
    // 3. Store in KV store
    raf_kv_store *store = raf_kv_create(16);
    raf_kv_set(store, "matrix", mat->data, mat->rows * mat->cols * sizeof(raf_scalar));
    
    // 4. Compress and archive
    raf_zipraf_archive *archive = raf_zipraf_create();
    uint8_t data[] = "Important data";
    raf_zipraf_add_data(archive, "data.txt", data, sizeof(data));
    
    // 5. Create toroidal grid for simulation
    raf_toroid_2d *grid = raf_toroid_2d_create(10, 10);
    
    // 6. Use PRNG to initialize
    raf_prng prng;
    raf_prng_init(&prng, raf_timestamp());
    for (int i = 0; i < 100; i++) {
        int x = raf_prng_range(&prng, 10);
        int y = raf_prng_range(&prng, 10);
        raf_toroid_2d_set(grid, x, y, raf_prng_float(&prng));
    }
    
    // 7. Calculate checksums
    uint32_t crc = raf_zipraf_crc32(data, sizeof(data));
    uint32_t adler = raf_adler32(data, sizeof(data));
    
    printf("Data integrity - CRC32: 0x%08X, Adler32: 0x%08X\n", crc, adler);
    
    // Cleanup
    raf_matrix_destroy(mat);
    raf_kv_destroy(store);
    raf_zipraf_destroy(archive);
    raf_toroid_2d_destroy(grid);
    
    return 0;
}
```

---

## Best Practices

### Memory Management

1. **Always destroy created objects**: Every `*_create()` call should have a corresponding `*_destroy()` call
2. **Check return values**: Many functions return error codes or NULL on failure
3. **Use memory pools** for frequent small allocations
4. **Prefer stack allocation** when object size is known and small

### Performance Optimization

1. **Detect hardware features** at startup and use appropriate code paths
2. **Reuse objects** instead of creating/destroying repeatedly
3. **Use bitstream operations** for efficient bit-level data packing
4. **Leverage toroidal topology** for wrap-around algorithms

### Security

1. **Use secure memory functions** (`raf_memcmp_secure`, `raf_memzero_secure`) for sensitive data
2. **Validate all inputs** before processing
3. **Use checksums** (CRC32, Adler32) to verify data integrity
4. **Clear sensitive data** after use

### Debugging

1. **Use `raf_bitstack_print_state()`** to visualize bitstack states
2. **Track state with hashes** using `raf_bitstack_calc_hash()` or `raf_hash64()`
3. **Profile with timestamps** using `raf_timestamp()`
4. **Validate checksums** at data boundaries

---

## Building and Linking

### CMake Integration

```cmake
# Enable Rafaelia Baremetal module
set(RAFAELIA_BAREMETAL ON)

# Add to your CMakeLists.txt
target_link_libraries(your_target rafaelia-baremetal)
```

### Manual Compilation

```bash
# Compile with baremetal module
gcc -o myapp myapp.c \
    -I/path/to/rafaelia-baremetal \
    -L/path/to/build \
    -lrafaelia-baremetal \
    -lm
```

---

## License

Copyright (c) 2026 Rafael Melo Reis  
Licensed under MIT License - See LICENSE file

This module is part of the Rafaelia Baremetal extensions to llama.cpp and maintains compatibility with the MIT License of the original project.
