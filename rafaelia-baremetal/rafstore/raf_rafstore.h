/**
 * Rafaelia Baremetal - RAFSTORE Storage Management
 * 
 * Pure C implementation for efficient storage management and data structures.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_RAFSTORE_H
#define RAFAELIA_RAFSTORE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory block structure for storage management */
typedef struct raf_memory_block {
    void *data;
    size_t size;
    int is_free;
    struct raf_memory_block *next;
    struct raf_memory_block *prev;
} raf_memory_block;

/* RAFSTORE memory pool */
typedef struct {
    raf_memory_block *blocks;
    size_t total_size;
    size_t used_size;
    size_t block_count;
} raf_memory_pool;

/* Memory pool operations */
raf_memory_pool* raf_pool_create(size_t size);
void raf_pool_destroy(raf_memory_pool *pool);
void* raf_pool_alloc(raf_memory_pool *pool, size_t size);
void raf_pool_free(raf_memory_pool *pool, void *ptr);
void raf_pool_defragment(raf_memory_pool *pool);
size_t raf_pool_available(const raf_memory_pool *pool);

/* Ring buffer for streaming data */
typedef struct {
    uint8_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} raf_ring_buffer;

raf_ring_buffer* raf_ring_create(size_t capacity);
void raf_ring_destroy(raf_ring_buffer *ring);
int raf_ring_push(raf_ring_buffer *ring, uint8_t value);
int raf_ring_pop(raf_ring_buffer *ring, uint8_t *value);
size_t raf_ring_available(const raf_ring_buffer *ring);
int raf_ring_is_empty(const raf_ring_buffer *ring);
int raf_ring_is_full(const raf_ring_buffer *ring);

/* Key-value store */
typedef struct raf_kv_entry {
    char *key;
    void *value;
    size_t value_size;
    struct raf_kv_entry *next;
} raf_kv_entry;

typedef struct {
    raf_kv_entry **buckets;
    size_t num_buckets;
    size_t num_entries;
} raf_kv_store;

raf_kv_store* raf_kv_create(size_t num_buckets);
void raf_kv_destroy(raf_kv_store *store);
int raf_kv_set(raf_kv_store *store, const char *key, const void *value, size_t value_size);
int raf_kv_get(raf_kv_store *store, const char *key, void **value, size_t *value_size);
int raf_kv_delete(raf_kv_store *store, const char *key);
int raf_kv_exists(raf_kv_store *store, const char *key);

/* Hash function for key-value store */
uint32_t raf_hash_string(const char *str);

/* Cache structure with LRU eviction */
typedef struct raf_cache_entry {
    char *key;
    void *data;
    size_t data_size;
    uint64_t access_count;
    uint64_t last_access;
    struct raf_cache_entry *next;
    struct raf_cache_entry *prev;
} raf_cache_entry;

typedef struct {
    raf_cache_entry *entries;
    size_t capacity;
    size_t count;
    uint64_t access_counter;
} raf_lru_cache;

raf_lru_cache* raf_cache_create(size_t capacity);
void raf_cache_destroy(raf_lru_cache *cache);
int raf_cache_put(raf_lru_cache *cache, const char *key, const void *data, size_t size);
int raf_cache_get(raf_lru_cache *cache, const char *key, void **data, size_t *size);
void raf_cache_clear(raf_lru_cache *cache);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_RAFSTORE_H */
