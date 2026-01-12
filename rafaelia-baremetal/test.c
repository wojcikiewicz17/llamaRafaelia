/**
 * Rafaelia Baremetal - Simple Test
 * 
 * Basic tests for the Rafaelia Baremetal Module
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "core/raf_core.h"
#include "bitraf/raf_bitraf.h"
#include "zipraf/raf_zipraf.h"
#include "rafstore/raf_rafstore.h"
#include "toroid/raf_toroid.h"
#include "hardware/raf_hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int test_hardware(void) {
    printf("Testing hardware detection...\n");
    
    raf_cpu_features features;
    raf_detect_cpu_features(&features);
    
    assert(features.arch != RAF_ARCH_UNKNOWN);
    assert(features.num_cores > 0);
    assert(features.cache_line_size > 0);
    
    printf("  Architecture: %s\n", raf_arch_name(features.arch));
    printf("  Cores: %d\n", features.num_cores);
    printf("  PASS\n");
    
    return 0;
}

int test_matrix_operations(void) {
    printf("Testing matrix operations...\n");
    
    /* Test matrix creation */
    raf_matrix *m = raf_matrix_create(3, 3);
    assert(m != NULL);
    assert(m->rows == 3);
    assert(m->cols == 3);
    
    /* Test identity matrix */
    raf_matrix_identity(m);
    assert(raf_matrix_get(m, 0, 0) == 1.0f);
    assert(raf_matrix_get(m, 1, 1) == 1.0f);
    assert(raf_matrix_get(m, 2, 2) == 1.0f);
    assert(raf_matrix_get(m, 0, 1) == 0.0f);
    
    /* Test matrix operations */
    raf_matrix *a = raf_matrix_create(2, 2);
    raf_matrix *b = raf_matrix_create(2, 2);
    raf_matrix *result = raf_matrix_create(2, 2);
    
    raf_matrix_set(a, 0, 0, 1.0f);
    raf_matrix_set(a, 0, 1, 2.0f);
    raf_matrix_set(a, 1, 0, 3.0f);
    raf_matrix_set(a, 1, 1, 4.0f);
    
    raf_matrix_identity(b);
    raf_matrix_mul(a, b, result);
    
    /* Multiplying by identity should give original */
    assert(raf_matrix_get(result, 0, 0) == 1.0f);
    assert(raf_matrix_get(result, 0, 1) == 2.0f);
    
    raf_matrix_destroy(m);
    raf_matrix_destroy(a);
    raf_matrix_destroy(b);
    raf_matrix_destroy(result);
    
    printf("  PASS\n");
    return 0;
}

int test_vector_operations(void) {
    printf("Testing vector operations...\n");
    
    raf_vector *v1 = raf_vector_create(3);
    raf_vector *v2 = raf_vector_create(3);
    
    v1->data[0] = 1.0f;
    v1->data[1] = 2.0f;
    v1->data[2] = 3.0f;
    
    v2->data[0] = 4.0f;
    v2->data[1] = 5.0f;
    v2->data[2] = 6.0f;
    
    /* Test dot product: 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32 */
    raf_scalar dot = raf_vector_dot(v1, v2);
    assert(dot == 32.0f);
    (void)dot; /* Used in assert */
    
    raf_vector_destroy(v1);
    raf_vector_destroy(v2);
    
    printf("  PASS\n");
    return 0;
}

int test_bitraf_operations(void) {
    printf("Testing BITRAF operations...\n");
    
    int result;
    
    /* Test bitstream */
    raf_bitstream *bs = raf_bitstream_create(1024);
    assert(bs != NULL);
    
    /* Write and read back */
    raf_bitstream_write_bits(bs, 0xAB, 8);
    raf_bitstream_write_bits(bs, 0x1234, 16);
    
    /* Test reset: should rewind without clearing data */
    raf_bitstream_reset(bs);
    uint32_t val1, val2;
    raf_bitstream_read_bits(bs, &val1, 8);
    raf_bitstream_read_bits(bs, &val2, 16);
    
    assert(val1 == 0xAB);
    assert(val2 == 0x1234);
    
    /* Test clear: should zero buffer */
    raf_bitstream_clear(bs);
    raf_bitstream_read_bits(bs, &val1, 8);
    assert(val1 == 0x00);
    
    /* Test bit packing */
    uint8_t input[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t packed[4];
    uint8_t unpacked[8];
    
    raf_bitraf_pack_4bit(input, 8, packed);
    raf_bitraf_unpack_4bit(packed, 4, unpacked);
    
    assert(unpacked[0] == 1);
    assert(unpacked[7] == 8);
    
    /* Test RLE encoding/decoding */
    uint8_t test_data[] = {0xFF, 0x00, 0xF0};  /* 8 ones, 8 zeros, 4 ones, 4 zeros */
    raf_bit_run runs[10];
    size_t num_runs = 10;
    
    result = raf_bitraf_encode_runs(test_data, 3, runs, &num_runs);
    assert(result == 0);
    assert(num_runs == 4);
    assert(runs[0].bit_value == 1 && runs[0].run_length == 8);
    assert(runs[1].bit_value == 0 && runs[1].run_length == 8);
    assert(runs[2].bit_value == 1 && runs[2].run_length == 4);
    assert(runs[3].bit_value == 0 && runs[3].run_length == 4);
    
    /* Test RLE decoding - round trip */
    uint8_t decoded[3];
    result = raf_bitraf_decode_runs(runs, num_runs, decoded, 3);
    assert(result == 0);
    assert(decoded[0] == 0xFF);
    assert(decoded[1] == 0x00);
    assert(decoded[2] == 0xF0);
    
    /* Test edge case: single bit */
    uint8_t single_bit[] = {0x80};  /* 1 one, 7 zeros */
    num_runs = 10;
    result = raf_bitraf_encode_runs(single_bit, 1, runs, &num_runs);
    assert(result == 0);
    assert(num_runs == 2);
    assert(runs[0].bit_value == 1 && runs[0].run_length == 1);
    assert(runs[1].bit_value == 0 && runs[1].run_length == 7);
    
    /* Test overflow protection: buffer too small */
    num_runs = 1;  /* Only space for 1 run */
    result = raf_bitraf_encode_runs(test_data, 3, runs, &num_runs);
    assert(result == -1);  /* Should fail */
    (void)result; /* Suppress unused warning - result is used in asserts */
    
    raf_bitstream_destroy(bs);
    
    printf("  PASS\n");
    return 0;
}

int test_toroid_operations(void) {
    printf("Testing TOROID operations...\n");
    
    raf_toroid_2d *toroid = raf_toroid_2d_create(10, 10);
    assert(toroid != NULL);
    
    /* Test wrapping */
    raf_toroid_2d_set(toroid, 0, 0, 1.0f);
    raf_toroid_2d_set(toroid, -1, -1, 2.0f);
    
    float val = raf_toroid_2d_get(toroid, 9, 9);
    assert(val == 2.0f);
    (void)val; /* Used in assert */
    
    /* Test neighbors */
    raf_toroid_2d_set(toroid, 5, 5, 10.0f);
    float neighbors[8];
    uint32_t count;
    raf_toroid_2d_get_neighbors(toroid, 5, 5, neighbors, &count);
    assert(count == 8);
    
    raf_toroid_2d_destroy(toroid);
    
    printf("  PASS\n");
    return 0;
}

int test_rafstore_operations(void) {
    printf("Testing RAFSTORE operations...\n");
    
    /* Test key-value store */
    raf_kv_store *store = raf_kv_create(256);
    assert(store != NULL);
    
    int value = 42;
    raf_kv_set(store, "test", &value, sizeof(value));
    
    void *retrieved;
    size_t size;
    int result = raf_kv_get(store, "test", &retrieved, &size);
    assert(result == 0);
    assert(*(int*)retrieved == 42);
    assert(size == sizeof(int));
    (void)result; /* Used in assert */
    
    /* Test ring buffer */
    raf_ring_buffer *ring = raf_ring_create(8);
    assert(ring != NULL);
    
    for (uint8_t i = 0; i < 5; i++) {
        raf_ring_push(ring, i);
    }
    
    uint8_t val;
    raf_ring_pop(ring, &val);
    assert(val == 0);
    
    raf_kv_destroy(store);
    raf_ring_destroy(ring);
    
    printf("  PASS\n");
    return 0;
}

int test_zipraf_operations(void) {
    printf("Testing ZIPRAF operations...\n");
    
    /* Test CRC32 */
    uint8_t data[] = "Hello, World!";
    uint32_t crc = raf_zipraf_crc32(data, strlen((char*)data));
    assert(crc != 0);
    (void)crc; /* Used in assert */
    
    /* Test archive */
    raf_zipraf_archive *archive = raf_zipraf_create();
    assert(archive != NULL);
    
    uint8_t test_data[] = {1, 2, 3, 4, 5};
    int result = raf_zipraf_add_data(archive, "test.dat", test_data, 5);
    assert(result == 0);
    (void)result; /* Used in assert */
    
    uint8_t *output;
    size_t size;
    result = raf_zipraf_extract_data(archive, "test.dat", &output, &size);
    assert(result == 0);
    assert(size == 5);
    assert(memcmp(output, test_data, 5) == 0);
    
    free(output);
    raf_zipraf_destroy(archive);
    
    printf("  PASS\n");
    return 0;
}

int main(void) {
    printf("\n");
    printf("====================================\n");
    printf("Rafaelia Baremetal Module - Tests\n");
    printf("====================================\n\n");
    
    int failures = 0;
    
    failures += test_hardware();
    failures += test_matrix_operations();
    failures += test_vector_operations();
    failures += test_bitraf_operations();
    failures += test_toroid_operations();
    failures += test_rafstore_operations();
    failures += test_zipraf_operations();
    
    printf("\n");
    if (failures == 0) {
        printf("====================================\n");
        printf("All tests PASSED!\n");
        printf("====================================\n");
        return 0;
    } else {
        printf("====================================\n");
        printf("Some tests FAILED! (%d failures)\n", failures);
        printf("====================================\n");
        return 1;
    }
}
