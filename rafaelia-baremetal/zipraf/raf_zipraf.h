/**
 * Rafaelia Baremetal - ZIPRAF Archiving and Compression
 * 
 * Pure C implementation for data archiving and compression utilities.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_ZIPRAF_H
#define RAFAELIA_ZIPRAF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ZIPRAF archive entry */
typedef struct {
    char name[256];
    uint32_t original_size;
    uint32_t compressed_size;
    uint32_t offset;
    uint32_t crc32;
} raf_zipraf_entry;

/* ZIPRAF archive structure */
typedef struct {
    uint32_t num_entries;
    raf_zipraf_entry *entries;
    uint8_t *data;
    size_t data_size;
} raf_zipraf_archive;

/* Archive creation and destruction */
raf_zipraf_archive* raf_zipraf_create(void);
void raf_zipraf_destroy(raf_zipraf_archive *archive);

/* Add entries to archive */
int raf_zipraf_add_data(raf_zipraf_archive *archive, const char *name,
                        const uint8_t *data, size_t size);
int raf_zipraf_add_file(raf_zipraf_archive *archive, const char *name,
                        const char *filename);

/* Extract from archive */
int raf_zipraf_extract_data(const raf_zipraf_archive *archive, const char *name,
                             uint8_t **output, size_t *size);
int raf_zipraf_list_entries(const raf_zipraf_archive *archive);

/* Save/Load archive */
int raf_zipraf_save(const raf_zipraf_archive *archive, const char *filename);
raf_zipraf_archive* raf_zipraf_load(const char *filename);

/* CRC32 calculation for integrity */
uint32_t raf_zipraf_crc32(const uint8_t *data, size_t size);

/* LZ77-based compression */
typedef struct {
    uint16_t offset;
    uint16_t length;
    uint8_t next_char;
} raf_lz77_token;

int raf_zipraf_lz77_compress(const uint8_t *input, size_t input_size,
                              uint8_t *output, size_t *output_size);
int raf_zipraf_lz77_decompress(const uint8_t *input, size_t input_size,
                                uint8_t *output, size_t *output_size);

/* Huffman coding structures */
typedef struct {
    uint8_t symbol;
    uint32_t frequency;
} raf_huffman_symbol;

typedef struct {
    uint8_t symbol;
    uint32_t code;
    uint8_t code_length;
} raf_huffman_code;

int raf_zipraf_huffman_compress(const uint8_t *input, size_t input_size,
                                 uint8_t *output, size_t *output_size);
int raf_zipraf_huffman_decompress(const uint8_t *input, size_t input_size,
                                   uint8_t *output, size_t *output_size);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_ZIPRAF_H */
