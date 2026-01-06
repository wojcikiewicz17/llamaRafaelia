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
    
    /* Simplified compression: just copy for now, actual algorithm can be added */
    compressed->original_size = size;
    compressed->compressed_size = size;
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
        for (int j = 0; j < 4 && (i + j) < input_size; j++) {
            bits |= ((input[i + j] & 0x03) << (6 - j * 2));
        }
        output[i / 4] = bits;
    }
}

void raf_bitraf_unpack_2bit(const uint8_t *input, size_t input_size, uint8_t *output) {
    for (size_t i = 0; i < input_size; i++) {
        for (int j = 0; j < 4; j++) {
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
