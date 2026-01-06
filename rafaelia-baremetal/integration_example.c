/**
 * Rafaelia Baremetal - Comprehensive Integration Example
 * 
 * Demonstrates how all modules work together in real-world scenarios
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "core/raf_core.h"
#include "bitraf/raf_bitraf.h"
#include "bitstack/raf_bitstack.h"
#include "zipraf/raf_zipraf.h"
#include "rafstore/raf_rafstore.h"
#include "toroid/raf_toroid.h"
#include "hardware/raf_hardware.h"
#include "utils/raf_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * Scenario 1: Matrix Computation with Storage
 * Uses: CORE (matrices), RAFSTORE (KV storage), HARDWARE (optimization)
 */
void scenario_matrix_computation_with_cache(void) {
    printf("\n=== Scenario 1: Matrix Computation with Storage ===\n");
    
    /* Detect hardware features for optimization */
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    printf("Using %s architecture with %d cores\n", 
           raf_arch_name(features.arch), features.num_cores);
    
    /* Create matrices */
    raf_matrix *a = raf_matrix_create(3, 3);
    raf_matrix *b = raf_matrix_create(3, 3);
    raf_matrix *result = raf_matrix_create(3, 3);
    
    /* Initialize with identity matrices */
    raf_matrix_identity(a);
    raf_matrix_identity(b);
    
    /* Scale matrix A */
    raf_matrix *a_scaled = raf_matrix_create(3, 3);
    raf_matrix_scale(a, 2.0f, a_scaled);
    
    /* Create KV store for frequently accessed results */
    raf_kv_store *store = raf_kv_create(16);
    
    /* Store result in KV store */
    char key_str[] = "matrix_3x3_scale_2";
    raf_kv_set(store, key_str, a_scaled->data, a_scaled->rows * a_scaled->cols * sizeof(raf_scalar));
    
    /* Multiply matrices */
    raf_matrix_mul(a_scaled, b, result);
    
    printf("Matrix computation completed\n");
    printf("Result[0,0] = %.2f (expected 2.00)\n", result->data[0]);
    
    /* Retrieve from store */
    void *retrieved_data;
    size_t retrieved_size;
    if (raf_kv_get(store, key_str, &retrieved_data, &retrieved_size) == 0) {
        printf("Successfully retrieved from store: %zu bytes\n", retrieved_size);
    }
    
    /* Cleanup */
    raf_matrix_destroy(a);
    raf_matrix_destroy(a_scaled);
    raf_matrix_destroy(b);
    raf_matrix_destroy(result);
    raf_kv_destroy(store);
}

/**
 * Scenario 2: Data Compression Pipeline
 * Uses: BITRAF (bit operations), ZIPRAF (compression), UTILS (checksums)
 */
void scenario_data_compression_pipeline(void) {
    printf("\n=== Scenario 2: Data Compression Pipeline ===\n");
    
    /* Original data */
    const char *original = "This is a test message for compression. "
                          "It contains repeated patterns. "
                          "It contains repeated patterns.";
    size_t original_len = strlen(original);
    
    printf("Original data: %zu bytes\n", original_len);
    
    /* Calculate checksum before compression */
    uint32_t original_crc = raf_zipraf_crc32((uint8_t*)original, original_len);
    printf("Original CRC32: 0x%08X\n", original_crc);
    
    /* Create archive */
    raf_zipraf_archive *archive = raf_zipraf_create();
    
    /* Add data to archive */
    raf_zipraf_add_data(archive, "message.txt", (uint8_t*)original, original_len);
    
    /* Extract and verify */
    uint8_t *extracted;
    size_t extracted_len;
    if (raf_zipraf_extract_data(archive, "message.txt", &extracted, &extracted_len) == 0) {
        uint32_t extracted_crc = raf_zipraf_crc32(extracted, extracted_len);
        printf("Extracted CRC32: 0x%08X\n", extracted_crc);
        printf("Data integrity: %s\n", 
               (original_crc == extracted_crc) ? "VERIFIED" : "FAILED");
        free(extracted);
    }
    
    /* Use bitstream for efficient bit-level encoding */
    raf_bitstream *bs = raf_bitstream_create(256);
    
    /* Pack 4-bit values efficiently */
    uint8_t values[] = {0x3, 0x7, 0xA, 0xF};
    for (int i = 0; i < 4; i++) {
        raf_bitstream_write_bits(bs, values[i], 4);
    }
    
    printf("Packed %d 4-bit values efficiently\n", 4);
    raf_bitstream_destroy(bs);
    
    raf_zipraf_destroy(archive);
}

/**
 * Scenario 3: Spatial Data Processing
 * Uses: TOROID (topology), CORE (vectors), UTILS (random generation)
 */
void scenario_spatial_data_processing(void) {
    printf("\n=== Scenario 3: Spatial Data Processing ===\n");
    
    /* Create 2D toroidal grid for game of life or heat diffusion */
    uint32_t width = 20, height = 20;
    raf_toroid_2d *grid_current = raf_toroid_2d_create(width, height);
    
    /* Initialize with random pattern using utils PRNG */
    raf_prng prng;
    raf_prng_init(&prng, 12345);
    
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            float value = raf_prng_float(&prng);
            raf_toroid_2d_set(grid_current, x, y, value);
        }
    }
    
    printf("Initialized %dx%d toroidal grid\n", width, height);
    
    /* Perform diffusion simulation (heat spread) - create destination grid */
    raf_toroid_2d *grid_next = raf_toroid_2d_create(width, height);
    for (int iter = 0; iter < 5; iter++) {
        raf_toroid_2d_diffuse(grid_current, grid_next, 0.25f);
        /* Swap grids for next iteration */
        raf_toroid_2d *temp = grid_current;
        grid_current = grid_next;
        grid_next = temp;
    }
    raf_toroid_2d_destroy(grid_next);
    
    /* Calculate statistics using vector operations */
    raf_vector *data_vec = raf_vector_create(width * height);
    for (uint32_t i = 0; i < width * height; i++) {
        data_vec->data[i] = grid_current->data[i];
    }
    
    /* Find min/max manually */
    float min_val = data_vec->data[0];
    float max_val = data_vec->data[0];
    for (uint32_t i = 1; i < width * height; i++) {
        if (data_vec->data[i] < min_val) min_val = data_vec->data[i];
        if (data_vec->data[i] > max_val) max_val = data_vec->data[i];
    }
    
    printf("After diffusion - Min: %.4f, Max: %.4f\n", min_val, max_val);
    
    /* Test toroidal distance */
    float dist = raf_toroid_2d_distance(grid_current, 0, 0, width-1, height-1);
    printf("Toroidal distance corner-to-corner: %.2f\n", dist);
    
    raf_vector_destroy(data_vec);
    raf_toroid_2d_destroy(grid_current);
}

/**
 * Scenario 4: Non-linear Logic and State Management
 * Uses: BITSTACK (non-linear logic), RAFSTORE (state storage), UTILS (hashing)
 */
void scenario_nonlinear_logic_and_state(void) {
    printf("\n=== Scenario 4: Non-linear Logic and State Management ===\n");
    
    /* Create bitstack for logical operations */
    raf_bitstack_state *state = raf_bitstack_create(4, 64);
    printf("Created bitstack with 4 layers, 64 bits per layer\n");
    
    /* Set initial pattern */
    for (uint32_t i = 0; i < 8; i++) {
        raf_bitstack_set_bit(state, 0, i, 1);
    }
    
    /* Calculate initial hash for state tracking */
    uint64_t initial_hash = raf_bitstack_calc_hash(state);
    printf("Initial state hash: 0x%016llX\n", (unsigned long long)initial_hash);
    
    /* Create KV store for state snapshots */
    raf_kv_store *store = raf_kv_create(16);
    
    /* Save state snapshot (store POD value only) */
    char snapshot_key[32];
    snprintf(snapshot_key, sizeof(snapshot_key), "state_snapshot_%llu", 
             (unsigned long long)initial_hash);
    raf_kv_set(store, snapshot_key, &initial_hash, sizeof(initial_hash));
    
    /* Perform single-flip operation */
    raf_bitstack_flip_single(state, 1, 32);
    printf("Flipped bit at layer 1, position 32\n");
    
    /* Measure opportunities for optimization */
    uint32_t test_positions[] = {10, 20, 30, 40, 50};
    raf_calc_result *opportunities = raf_calc_result_create(10);
    
    raf_bitstack_measure_opportunities(state, opportunities, test_positions, 5);
    printf("Found %zu optimization opportunities\n", opportunities->num_opportunities);
    
    if (opportunities->num_opportunities > 0) {
        printf("Best opportunity: position %u with confidence %.2f\n",
               opportunities->opportunities[0].bit_positions[0],
               opportunities->opportunities[0].confidence);
    }
    
    /* Cleanup */
    raf_calc_result_destroy(opportunities);
    raf_kv_destroy(store);
    raf_bitstack_destroy(state);
}

/**
 * Scenario 5: Performance-Critical Path
 * Uses: All modules for a complete optimization pipeline
 */
void scenario_performance_critical_pipeline(void) {
    printf("\n=== Scenario 5: Performance-Critical Pipeline ===\n");
    
    /* Start timing */
    uint64_t start_time = raf_timestamp();
    
    /* Create memory pool for efficient allocations */
    raf_memory_pool *pool = raf_pool_create(4096);
    
    /* Allocate multiple small objects from pool */
    void *obj1 = raf_pool_alloc(pool, 64);
    void *obj2 = raf_pool_alloc(pool, 128);
    void *obj3 = raf_pool_alloc(pool, 256);
    
    /* Use secure memory operations from utils */
    uint8_t sensitive_data1[32] = "secret_key_1";
    uint8_t sensitive_data2[32] = "secret_key_2";
    
    /* Secure comparison (timing-attack resistant) */
    int are_equal = raf_memcmp_secure(sensitive_data1, sensitive_data2, 32);
    printf("Secure comparison result: %s\n", are_equal == 0 ? "Equal" : "Different");
    
    /* Securely zero sensitive data */
    raf_memzero_secure(sensitive_data1, sizeof(sensitive_data1));
    raf_memzero_secure(sensitive_data2, sizeof(sensitive_data2));
    
    /* Use ring buffer for streaming data */
    raf_ring_buffer *ring = raf_ring_create(1024);
    
    /* Write data stream */
    for (int i = 0; i < 100; i++) {
        raf_ring_push(ring, (uint8_t)i);
    }
    
    /* Read back */
    int read_count = 0;
    while (!raf_ring_is_empty(ring)) {
        uint8_t value;
        raf_ring_pop(ring, &value);
        read_count++;
    }
    
    printf("Processed %d items through ring buffer\n", read_count);
    
    /* Calculate various hashes for data integrity */
    const char *data = "Performance test data";
    /* Use available hash functions */
    uint32_t adler = raf_adler32((uint8_t*)data, strlen(data));
    uint64_t hash = raf_hash64((uint8_t*)data, strlen(data));
    
    printf("Checksums - Adler32: 0x%08X, Hash64: 0x%016llX\n",
           adler, (unsigned long long)hash);
    
    /* End timing */
    uint64_t end_time = raf_timestamp();
    printf("Pipeline completed in %llu time units\n", 
           (unsigned long long)(end_time - start_time));
    
    /* Cleanup */
    /* Pool allocations are automatically freed with pool */
    (void)obj1;
    (void)obj2;
    (void)obj3;
    raf_ring_destroy(ring);
    raf_pool_destroy(pool);
}

/**
 * Scenario 6: Base64 Encoding with Archive Storage
 * Uses: UTILS (encoding), ZIPRAF (archiving), RAFSTORE (storage)
 */
void scenario_encoding_and_storage(void) {
    printf("\n=== Scenario 6: Encoding and Storage ===\n");
    
    /* Binary data to encode */
    uint8_t binary_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    size_t binary_len = sizeof(binary_data);
    
    /* Encode to base64 */
    char encoded[256];
    size_t encoded_len = raf_base64_encode(binary_data, binary_len, 
                                          encoded, sizeof(encoded));
    printf("Base64 encoded: %s (%zu bytes)\n", encoded, encoded_len);
    
    /* Encode to hexadecimal */
    char hex_encoded[256];
    size_t hex_len = raf_hex_encode(binary_data, binary_len, 
                                    hex_encoded, sizeof(hex_encoded));
    printf("Hex encoded: %s (%zu bytes)\n", hex_encoded, hex_len);
    
    /* Store both encodings in archive */
    raf_zipraf_archive *archive = raf_zipraf_create();
    raf_zipraf_add_data(archive, "data.b64", (uint8_t*)encoded, encoded_len);
    raf_zipraf_add_data(archive, "data.hex", (uint8_t*)hex_encoded, hex_len);
    
    int num_entries = raf_zipraf_list_entries(archive);
    printf("Archive contains %d entries\n", num_entries);
    
    /* Extract and verify hex encoding */
    uint8_t *extracted;
    size_t extracted_len;
    if (raf_zipraf_extract_data(archive, "data.hex", &extracted, &extracted_len) == 0) {
        printf("Extracted hex data: %zu bytes\n", extracted_len);
        printf("Extraction verification: SUCCESS\n");
        free(extracted);
    }
    
    raf_zipraf_destroy(archive);
}

int main(void) {
    printf("=========================================\n");
    printf("Rafaelia Baremetal Integration Examples\n");
    printf("=========================================\n");
    
    scenario_matrix_computation_with_cache();
    scenario_data_compression_pipeline();
    scenario_spatial_data_processing();
    scenario_nonlinear_logic_and_state();
    scenario_performance_critical_pipeline();
    scenario_encoding_and_storage();
    
    printf("\n=========================================\n");
    printf("All Integration Scenarios Completed!\n");
    printf("=========================================\n");
    
    return 0;
}
