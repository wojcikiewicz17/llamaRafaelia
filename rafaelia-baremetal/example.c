/**
 * Rafaelia Baremetal - Example Usage
 * 
 * Demonstrates the core features of the Rafaelia Baremetal Module
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "core/raf_core.h"
#include "core/raf_system.h"
#include "bitraf/raf_bitraf.h"
#include "zipraf/raf_zipraf.h"
#include "rafstore/raf_rafstore.h"
#include "toroid/raf_toroid.h"
#include "hardware/raf_hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void example_hardware_detection(void) {
    printf("\n=== Hardware Detection ===\n");
    
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    
    printf("Architecture: %s\n", raf_arch_name(features.arch));
    printf("Number of cores: %d\n", features.num_cores);
    printf("Cache line size: %u bytes\n", features.cache_line_size);
    printf("SSE: %s\n", features.has_sse ? "Yes" : "No");
    printf("AVX: %s\n", features.has_avx ? "Yes" : "No");
    printf("AVX2: %s\n", features.has_avx2 ? "Yes" : "No");
    printf("NEON: %s\n", features.has_neon ? "Yes" : "No");
}

void example_system_identity(void) {
    printf("\n=== Rafaelia System Identity ===\n");

    const raf_system_identity *identity = raf_system_identity_get();

    printf("RF_ID: %s\n", identity->rf_id);
    printf("Kernel: %s\n", identity->kernel);
    printf("Mode: %s\n", identity->mode);
    printf("Ethic: %s\n", identity->ethic);
    printf("Hash Core: %s\n", identity->hash_core);
    printf("Vector Core: %s\n", identity->vector_core);
    printf("Cognition: %s\n", identity->cognition);
    printf("Universe: %s\n", identity->universe);
}

void example_system_modules(void) {
    printf("\n=== Rafaelia Architecture Map ===\n");

    const raf_module_info *modules = NULL;
    unsigned int count = raf_system_list_modules(&modules);

    for (raf_layer layer = RAF_LAYER_HIGH; layer <= RAF_LAYER_HAL; layer++) {
        printf("%s:\n", raf_layer_name(layer));
        for (unsigned int i = 0; i < count; i++) {
            if (modules[i].layer == layer) {
                printf("  - %s: %s\n", modules[i].name, modules[i].description);
            }
        }
    }
}

void example_matrix_operations(void) {
    printf("\n=== Matrix Operations ===\n");
    
    /* Create matrices */
    raf_matrix *a = raf_matrix_create(3, 3);
    raf_matrix *b = raf_matrix_create(3, 3);
    raf_matrix *result = raf_matrix_create(3, 3);
    
    /* Fill with test data */
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            raf_matrix_set(a, i, j, i + j + 1.0f);
            raf_matrix_set(b, i, j, (i + 1.0f) * (j + 1.0f));
        }
    }
    
    /* Matrix multiplication */
    raf_matrix_mul(a, b, result);
    
    printf("Matrix A x B result[0,0] = %.2f\n", raf_matrix_get(result, 0, 0));
    
    /* Cleanup */
    raf_matrix_destroy(a);
    raf_matrix_destroy(b);
    raf_matrix_destroy(result);
}

void example_bitraf_operations(void) {
    printf("\n=== BITRAF Operations ===\n");
    
    /* Create bitstream */
    raf_bitstream *bs = raf_bitstream_create(1024);
    
    /* Write some bits */
    raf_bitstream_write_bits(bs, 0xAB, 8);
    raf_bitstream_write_bits(bs, 0x1234, 16);
    
    /* Reset and read back */
    raf_bitstream_reset(bs);
    uint32_t value1, value2;
    raf_bitstream_read_bits(bs, &value1, 8);
    raf_bitstream_read_bits(bs, &value2, 16);
    
    printf("Read values: 0x%X, 0x%X\n", value1, value2);
    
    /* Test bit packing */
    uint8_t input[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t packed[4];
    uint8_t unpacked[8];
    
    raf_bitraf_pack_4bit(input, 8, packed);
    raf_bitraf_unpack_4bit(packed, 4, unpacked);
    
    printf("4-bit packing test passed: %s\n", 
           (unpacked[0] == 1 && unpacked[7] == 8) ? "Yes" : "No");
    
    raf_bitstream_destroy(bs);
}

void example_toroid_operations(void) {
    printf("\n=== TOROID Operations ===\n");
    
    /* Create 2D toroid */
    raf_toroid_2d *toroid = raf_toroid_2d_create(10, 10);
    
    /* Set some values */
    raf_toroid_2d_set(toroid, 0, 0, 1.0f);
    raf_toroid_2d_set(toroid, 5, 5, 2.0f);
    
    /* Test wrapping - setting at -1 should wrap to 9 */
    raf_toroid_2d_set(toroid, -1, -1, 3.0f);
    float wrapped = raf_toroid_2d_get(toroid, 9, 9);
    
    printf("Toroid wrapping test: value at (-1,-1) = value at (9,9) = %.1f\n", wrapped);
    
    /* Test distance calculation */
    float dist = raf_toroid_2d_distance(toroid, 0, 0, 9, 9);
    printf("Toroidal distance from (0,0) to (9,9): %.2f\n", dist);
    
    raf_toroid_2d_destroy(toroid);
}

void example_rafstore_operations(void) {
    printf("\n=== RAFSTORE Operations ===\n");
    
    /* Create key-value store */
    raf_kv_store *store = raf_kv_create(256);
    
    /* Store some values */
    int value1 = 42;
    float value2 = 3.14f;
    char value3[] = "Hello, Rafaelia!";
    
    raf_kv_set(store, "answer", &value1, sizeof(value1));
    raf_kv_set(store, "pi", &value2, sizeof(value2));
    raf_kv_set(store, "message", value3, strlen(value3) + 1);
    
    /* Retrieve values */
    void *retrieved;
    size_t size;
    
    if (raf_kv_get(store, "answer", &retrieved, &size) == 0) {
        printf("Retrieved 'answer': %d\n", *(int*)retrieved);
    }
    
    if (raf_kv_get(store, "message", &retrieved, &size) == 0) {
        printf("Retrieved 'message': %s\n", (char*)retrieved);
    }
    
    /* Test ring buffer */
    raf_ring_buffer *ring = raf_ring_create(8);
    
    for (uint8_t i = 0; i < 5; i++) {
        raf_ring_push(ring, i);
    }
    
    uint8_t val;
    raf_ring_pop(ring, &val);
    printf("Ring buffer first value: %u\n", val);
    
    raf_kv_destroy(store);
    raf_ring_destroy(ring);
}

int main(void) {
    printf("Rafaelia Baremetal Module - Example Usage\n");
    printf("==========================================\n");

    example_system_identity();
    example_system_modules();
    example_hardware_detection();
    example_matrix_operations();
    example_bitraf_operations();
    example_toroid_operations();
    example_rafstore_operations();
    
    printf("\n=== All Examples Completed Successfully ===\n");
    
    return 0;
}
