/**
 * Rafaelia Baremetal - BITRAF Bit-Level Operations
 * 
 * Pure C implementation for bit-level compression and manipulation.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_BITRAF_H
#define RAFAELIA_BITRAF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bit stream structure for efficient bit manipulation */
typedef struct {
    uint8_t *buffer;
    size_t buffer_size;
    size_t byte_pos;
    uint8_t bit_pos;
} raf_bitstream;

/* Bit-level operations */
raf_bitstream* raf_bitstream_create(size_t buffer_size);
void raf_bitstream_destroy(raf_bitstream *bs);
void raf_bitstream_reset(raf_bitstream *bs);

/* Write operations */
int raf_bitstream_write_bit(raf_bitstream *bs, uint8_t bit);
int raf_bitstream_write_bits(raf_bitstream *bs, uint32_t value, uint8_t num_bits);
int raf_bitstream_write_byte(raf_bitstream *bs, uint8_t byte);

/* Read operations */
int raf_bitstream_read_bit(raf_bitstream *bs, uint8_t *bit);
int raf_bitstream_read_bits(raf_bitstream *bs, uint32_t *value, uint8_t num_bits);
int raf_bitstream_read_byte(raf_bitstream *bs, uint8_t *byte);

/* Bit manipulation utilities */
uint8_t raf_count_bits(uint32_t value);
uint8_t raf_count_leading_zeros(uint32_t value);
uint8_t raf_count_trailing_zeros(uint32_t value);
uint32_t raf_reverse_bits(uint32_t value, uint8_t num_bits);

/* BITRAF compression structures */
typedef struct {
    uint32_t original_size;
    uint32_t compressed_size;
    uint8_t *data;
} raf_bitraf_compressed;

/* BITRAF compression/decompression functions */
raf_bitraf_compressed* raf_bitraf_compress(const uint8_t *data, size_t size);
int raf_bitraf_decompress(const raf_bitraf_compressed *compressed, 
                           uint8_t *output, size_t output_size);
void raf_bitraf_compressed_destroy(raf_bitraf_compressed *compressed);

/* Run-length encoding for bit sequences */
typedef struct {
    uint32_t run_length;
    uint8_t bit_value;
} raf_bit_run;

int raf_bitraf_encode_runs(const uint8_t *data, size_t size, 
                            raf_bit_run *runs, size_t *num_runs);
int raf_bitraf_decode_runs(const raf_bit_run *runs, size_t num_runs,
                            uint8_t *output, size_t output_size);

/* Bit packing utilities for matrix quantization */
void raf_bitraf_pack_4bit(const uint8_t *input, size_t input_size, uint8_t *output);
void raf_bitraf_unpack_4bit(const uint8_t *input, size_t input_size, uint8_t *output);
void raf_bitraf_pack_2bit(const uint8_t *input, size_t input_size, uint8_t *output);
void raf_bitraf_unpack_2bit(const uint8_t *input, size_t input_size, uint8_t *output);

/* Delta encoding for sequential data */
void raf_bitraf_delta_encode(const int32_t *input, size_t size, int32_t *output);
void raf_bitraf_delta_decode(const int32_t *input, size_t size, int32_t *output);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_BITRAF_H */
