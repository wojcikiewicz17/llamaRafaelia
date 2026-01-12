/**
 * Rafaelia Baremetal - BITRAF Implementation
 * 
 * Pure C implementation for bit-level compression and manipulation.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_bitraf.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Bitstream operations */
raf_bitstream* raf_bitstream_create(size_t buffer_size) {
    raf_bitstream *bs = (raf_bitstream*)malloc(sizeof(raf_bitstream));
    if (!bs) return NULL;
    
    bs->buffer = (uint8_t*)calloc(buffer_size, 1);
    if (!bs->buffer) {
        free(bs);
        return NULL;
    }
    
    bs->buffer_size = buffer_size;
    bs->byte_pos = 0;
    bs->bit_pos = 0;
    
    return bs;
}

void raf_bitstream_destroy(raf_bitstream *bs) {
    if (bs) {
        if (bs->buffer) free(bs->buffer);
        free(bs);
    }
}

void raf_bitstream_reset(raf_bitstream *bs) {
    if (bs) {
        bs->byte_pos = 0;
        bs->bit_pos = 0;
    }
}

void raf_bitstream_clear(raf_bitstream *bs) {
    if (bs) {
        bs->byte_pos = 0;
        bs->bit_pos = 0;
        memset(bs->buffer, 0, bs->buffer_size);
    }
}

/* Write single bit */
int raf_bitstream_write_bit(raf_bitstream *bs, uint8_t bit) {
    if (!bs || bs->byte_pos >= bs->buffer_size) return -1;
    
    if (bit) {
        bs->buffer[bs->byte_pos] |= (1 << (7 - bs->bit_pos));
    }
    
    bs->bit_pos++;
    if (bs->bit_pos >= 8) {
        bs->bit_pos = 0;
        bs->byte_pos++;
    }
    
    return 0;
}

/* Write multiple bits */
int raf_bitstream_write_bits(raf_bitstream *bs, uint32_t value, uint8_t num_bits) {
    if (!bs || num_bits > 32) return -1;
    
    for (int i = num_bits - 1; i >= 0; i--) {
        uint8_t bit = (value >> i) & 1;
        if (raf_bitstream_write_bit(bs, bit) != 0) return -1;
    }
    
    return 0;
}

/* Write byte */
int raf_bitstream_write_byte(raf_bitstream *bs, uint8_t byte) {
    return raf_bitstream_write_bits(bs, byte, 8);
}

/* Read single bit */
int raf_bitstream_read_bit(raf_bitstream *bs, uint8_t *bit) {
    if (!bs || !bit || bs->byte_pos >= bs->buffer_size) return -1;
    
    *bit = (bs->buffer[bs->byte_pos] >> (7 - bs->bit_pos)) & 1;
    
    bs->bit_pos++;
    if (bs->bit_pos >= 8) {
        bs->bit_pos = 0;
        bs->byte_pos++;
    }
    
    return 0;
}

/* Read multiple bits */
int raf_bitstream_read_bits(raf_bitstream *bs, uint32_t *value, uint8_t num_bits) {
    if (!bs || !value || num_bits > 32) return -1;
    
    *value = 0;
    for (uint8_t i = 0; i < num_bits; i++) {
        uint8_t bit;
        if (raf_bitstream_read_bit(bs, &bit) != 0) return -1;
        *value = (*value << 1) | bit;
    }
    
    return 0;
}

/* Read byte */
int raf_bitstream_read_byte(raf_bitstream *bs, uint8_t *byte) {
    uint32_t value;
    int result = raf_bitstream_read_bits(bs, &value, 8);
    if (result == 0) *byte = (uint8_t)value;
    return result;
}

/* Bit manipulation utilities */
uint8_t raf_count_bits(uint32_t value) {
    uint8_t count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

uint8_t raf_count_leading_zeros(uint32_t value) {
    if (value == 0) return 32;
    
    uint8_t count = 0;
    uint32_t mask = 0x80000000;
    
    while ((value & mask) == 0) {
        count++;
        mask >>= 1;
    }
    
    return count;
}

uint8_t raf_count_trailing_zeros(uint32_t value) {
    if (value == 0) return 32;
    
    uint8_t count = 0;
    while ((value & 1) == 0) {
        count++;
        value >>= 1;
    }
    
    return count;
}

uint32_t raf_reverse_bits(uint32_t value, uint8_t num_bits) {
    uint32_t result = 0;
    for (uint8_t i = 0; i < num_bits; i++) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

/* Simple BITRAF compression (placeholder - can be enhanced) */
raf_bitraf_compressed* raf_bitraf_compress(const uint8_t *data, size_t size) {
    if (!data || size == 0) return NULL;
    
    raf_bitraf_compressed *compressed = (raf_bitraf_compressed*)malloc(sizeof(raf_bitraf_compressed));
    if (!compressed) return NULL;
    
    /* Check if size fits in uint32_t */
    if (size > UINT32_MAX) {
        free(compressed);
        return NULL;  /* Size too large for uint32_t */
    }
    
    /* Simplified compression: just copy for now, actual algorithm can be added */
    compressed->original_size = (uint32_t)size;
    compressed->compressed_size = (uint32_t)size;
    compressed->data = (uint8_t*)malloc(size);
    
    if (!compressed->data) {
        free(compressed);
        return NULL;
    }
    
    memcpy(compressed->data, data, size);
    
    return compressed;
}

int raf_bitraf_decompress(const raf_bitraf_compressed *compressed, 
                           uint8_t *output, size_t output_size) {
    if (!compressed || !output || output_size < compressed->original_size) return -1;
    
    memcpy(output, compressed->data, compressed->original_size);
    return 0;
}

void raf_bitraf_compressed_destroy(raf_bitraf_compressed *compressed) {
    if (compressed) {
        if (compressed->data) free(compressed->data);
        free(compressed);
    }
}

/* Bit packing for quantization */
void raf_bitraf_pack_4bit(const uint8_t *input, size_t input_size, uint8_t *output) {
    for (size_t i = 0; i < input_size; i += 2) {
        uint8_t high = input[i] & 0x0F;
        uint8_t low = (i + 1 < input_size) ? (input[i + 1] & 0x0F) : 0;
        output[i / 2] = (high << 4) | low;
    }
}

void raf_bitraf_unpack_4bit(const uint8_t *input, size_t input_size, uint8_t *output) {
    for (size_t i = 0; i < input_size; i++) {
        output[i * 2] = (input[i] >> 4) & 0x0F;
        output[i * 2 + 1] = input[i] & 0x0F;
    }
}

void raf_bitraf_pack_2bit(const uint8_t *input, size_t input_size, uint8_t *output) {
    for (size_t i = 0; i < input_size; i += 4) {
        uint8_t bits = 0;
        for (size_t j = 0; j < 4 && (i + j) < input_size; j++) {
            bits |= ((input[i + j] & 0x03) << (6 - j * 2));
        }
        output[i / 4] = bits;
    }
}

void raf_bitraf_unpack_2bit(const uint8_t *input, size_t input_size, uint8_t *output) {
    for (size_t i = 0; i < input_size; i++) {
        for (size_t j = 0; j < 4; j++) {
            output[i * 4 + j] = (input[i] >> (6 - j * 2)) & 0x03;
        }
    }
}

/* Delta encoding */
void raf_bitraf_delta_encode(const int32_t *input, size_t size, int32_t *output) {
    if (!input || !output || size == 0) return;
    
    output[0] = input[0];
    for (size_t i = 1; i < size; i++) {
        output[i] = input[i] - input[i - 1];
    }
}

void raf_bitraf_delta_decode(const int32_t *input, size_t size, int32_t *output) {
    if (!input || !output || size == 0) return;
    
    output[0] = input[0];
    for (size_t i = 1; i < size; i++) {
        output[i] = output[i - 1] + input[i];
    }
}

/* Run-length encoding for bit sequences */
int raf_bitraf_encode_runs(const uint8_t *data, size_t size, 
                            raf_bit_run *runs, size_t *num_runs) {
    if (!data || !runs || !num_runs || size == 0) return -1;
    
    size_t run_count = 0;
    size_t max_runs = *num_runs;
    
    /* Process each bit in the input data */
    uint8_t current_bit = (data[0] >> 7) & 1;
    uint32_t run_length = 0;
    
    for (size_t byte_idx = 0; byte_idx < size; byte_idx++) {
        for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
            uint8_t bit = (data[byte_idx] >> bit_idx) & 1;
            
            if (bit == current_bit) {
                run_length++;
                /* Check for overflow */
                if (run_length == UINT32_MAX) {
                    if (run_count >= max_runs) return -1;
                    runs[run_count].bit_value = current_bit;
                    runs[run_count].run_length = run_length;
                    run_count++;
                    run_length = 0;
                }
            } else {
                /* New run starts */
                if (run_count >= max_runs) return -1;
                runs[run_count].bit_value = current_bit;
                runs[run_count].run_length = run_length;
                run_count++;
                
                current_bit = bit;
                run_length = 1;
            }
        }
    }
    
    /* Store final run */
    if (run_length > 0) {
        if (run_count >= max_runs) return -1;
        runs[run_count].bit_value = current_bit;
        runs[run_count].run_length = run_length;
        run_count++;
    }
    
    *num_runs = run_count;
    return 0;
}

int raf_bitraf_decode_runs(const raf_bit_run *runs, size_t num_runs,
                            uint8_t *output, size_t output_size) {
    if (!runs || !output || num_runs == 0) return -1;
    
    /* Calculate total bits needed */
    uint64_t total_bits = 0;
    for (size_t i = 0; i < num_runs; i++) {
        total_bits += runs[i].run_length;
    }
    
    /* Check if output buffer is large enough */
    size_t bytes_needed = (total_bits + 7) / 8;
    if (output_size < bytes_needed) return -1;
    
    /* Initialize output buffer */
    memset(output, 0, output_size);
    
    /* Write runs to output */
    size_t byte_pos = 0;
    int bit_pos = 7;
    
    for (size_t run_idx = 0; run_idx < num_runs; run_idx++) {
        uint8_t bit_value = runs[run_idx].bit_value;
        uint32_t run_length = runs[run_idx].run_length;
        
        for (uint32_t i = 0; i < run_length; i++) {
            if (bit_value) {
                output[byte_pos] |= (1 << bit_pos);
            }
            
            bit_pos--;
            if (bit_pos < 0) {
                bit_pos = 7;
                byte_pos++;
                /* Only check bounds if we need to write more data */
                if (byte_pos >= output_size && (run_idx + 1 < num_runs || i + 1 < run_length)) {
                    return -1;
                }
            }
        }
    }
    
    return 0;
}
