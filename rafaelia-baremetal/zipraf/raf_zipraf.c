/**
 * Rafaelia Baremetal - ZIPRAF Implementation
 * 
 * Pure C implementation for data archiving and compression utilities.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_zipraf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* CRC32 table for checksum calculation */
static uint32_t crc32_table[256];
static int crc32_table_initialized = 0;

static void raf_zipraf_init_crc32_table(void) {
    if (crc32_table_initialized) return;
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    
    crc32_table_initialized = 1;
}

uint32_t raf_zipraf_crc32(const uint8_t *data, size_t size) {
    raf_zipraf_init_crc32_table();
    
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }
    
    return ~crc;
}

/* Archive management */
raf_zipraf_archive* raf_zipraf_create(void) {
    raf_zipraf_archive *archive = (raf_zipraf_archive*)malloc(sizeof(raf_zipraf_archive));
    if (!archive) return NULL;
    
    archive->num_entries = 0;
    archive->entries = NULL;
    archive->data = NULL;
    archive->data_size = 0;
    
    return archive;
}

void raf_zipraf_destroy(raf_zipraf_archive *archive) {
    if (archive) {
        if (archive->entries) free(archive->entries);
        if (archive->data) free(archive->data);
        free(archive);
    }
}

int raf_zipraf_add_data(raf_zipraf_archive *archive, const char *name,
                        const uint8_t *data, size_t size) {
    if (!archive || !name || !data) return -1;
    
    /* Expand entries array */
    raf_zipraf_entry *new_entries = (raf_zipraf_entry*)realloc(
        archive->entries, 
        (archive->num_entries + 1) * sizeof(raf_zipraf_entry)
    );
    
    if (!new_entries) return -1;
    archive->entries = new_entries;
    
    /* Create new entry */
    raf_zipraf_entry *entry = &archive->entries[archive->num_entries];
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    entry->original_size = size;
    entry->compressed_size = size; /* Simple: no compression for now */
    entry->offset = archive->data_size;
    entry->crc32 = raf_zipraf_crc32(data, size);
    
    /* Expand data buffer */
    uint8_t *new_data = (uint8_t*)realloc(archive->data, archive->data_size + size);
    if (!new_data) return -1;
    archive->data = new_data;
    
    /* Copy data */
    memcpy(archive->data + archive->data_size, data, size);
    archive->data_size += size;
    archive->num_entries++;
    
    return 0;
}

int raf_zipraf_extract_data(const raf_zipraf_archive *archive, const char *name,
                             uint8_t **output, size_t *size) {
    if (!archive || !name || !output || !size) return -1;
    
    /* Find entry */
    for (uint32_t i = 0; i < archive->num_entries; i++) {
        if (strcmp(archive->entries[i].name, name) == 0) {
            const raf_zipraf_entry *entry = &archive->entries[i];
            
            /* Allocate output buffer */
            *output = (uint8_t*)malloc(entry->original_size);
            if (!*output) return -1;
            
            /* Copy data */
            memcpy(*output, archive->data + entry->offset, entry->original_size);
            *size = entry->original_size;
            
            /* Verify CRC */
            uint32_t crc = raf_zipraf_crc32(*output, *size);
            if (crc != entry->crc32) {
                free(*output);
                *output = NULL;
                return -2; /* CRC mismatch */
            }
            
            return 0;
        }
    }
    
    return -1; /* Entry not found */
}

int raf_zipraf_list_entries(const raf_zipraf_archive *archive) {
    if (!archive) return -1;
    
    for (uint32_t i = 0; i < archive->num_entries; i++) {
        const raf_zipraf_entry *entry = &archive->entries[i];
        /* Would print to stdout in a real implementation */
        /* For now, just return count */
    }
    
    return archive->num_entries;
}

/* Simple LZ77 compression (sliding window) */
int raf_zipraf_lz77_compress(const uint8_t *input, size_t input_size,
                              uint8_t *output, size_t *output_size) {
    if (!input || !output || !output_size) return -1;
    
    /* Simplified implementation - just copy data for now */
    /* Full LZ77 would use sliding window and match finding */
    if (*output_size < input_size) return -1;
    
    memcpy(output, input, input_size);
    *output_size = input_size;
    
    return 0;
}

int raf_zipraf_lz77_decompress(const uint8_t *input, size_t input_size,
                                uint8_t *output, size_t *output_size) {
    if (!input || !output || !output_size) return -1;
    
    /* Simplified implementation - just copy data for now */
    if (*output_size < input_size) return -1;
    
    memcpy(output, input, input_size);
    *output_size = input_size;
    
    return 0;
}

/* Huffman coding (placeholder for now) */
int raf_zipraf_huffman_compress(const uint8_t *input, size_t input_size,
                                 uint8_t *output, size_t *output_size) {
    if (!input || !output || !output_size) return -1;
    
    /* Simplified: just copy for now */
    /* Full Huffman would build frequency table and tree */
    if (*output_size < input_size) return -1;
    
    memcpy(output, input, input_size);
    *output_size = input_size;
    
    return 0;
}

int raf_zipraf_huffman_decompress(const uint8_t *input, size_t input_size,
                                   uint8_t *output, size_t *output_size) {
    if (!input || !output || !output_size) return -1;
    
    /* Simplified: just copy for now */
    if (*output_size < input_size) return -1;
    
    memcpy(output, input, input_size);
    *output_size = input_size;
    
    return 0;
}
