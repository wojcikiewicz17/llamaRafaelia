/**
 * Rafaelia Baremetal - RAFSTORE Implementation
 * 
 * Pure C implementation for efficient storage management and data structures.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_rafstore.h"
#include <stdlib.h>
#include <string.h>

/* Memory Pool Implementation */
raf_memory_pool* raf_pool_create(size_t size) {
    raf_memory_pool *pool = (raf_memory_pool*)malloc(sizeof(raf_memory_pool));
    if (!pool) return NULL;
    
    pool->blocks = (raf_memory_block*)malloc(sizeof(raf_memory_block));
    if (!pool->blocks) {
        free(pool);
        return NULL;
    }
    
    pool->blocks->data = malloc(size);
    if (!pool->blocks->data) {
        free(pool->blocks);
        free(pool);
        return NULL;
    }
    
    pool->blocks->size = size;
    pool->blocks->is_free = 1;
    pool->blocks->next = NULL;
    pool->blocks->prev = NULL;
    
    pool->total_size = size;
    pool->used_size = 0;
    pool->block_count = 1;
    
    return pool;
}

void raf_pool_destroy(raf_memory_pool *pool) {
    if (!pool) return;
    
    raf_memory_block *current = pool->blocks;
    while (current) {
        raf_memory_block *next = current->next;
        if (current->data) free(current->data);
        free(current);
        current = next;
    }
    
    free(pool);
}

void* raf_pool_alloc(raf_memory_pool *pool, size_t size) {
    if (!pool || size == 0) return NULL;
    
    /* Find first free block that fits */
    raf_memory_block *current = pool->blocks;
    while (current) {
        if (current->is_free && current->size >= size) {
            current->is_free = 0;
            pool->used_size += current->size;
            return current->data;
        }
        current = current->next;
    }
    
    return NULL; /* No suitable block found */
}

void raf_pool_free(raf_memory_pool *pool, void *ptr) {
    if (!pool || !ptr) return;
    
    raf_memory_block *current = pool->blocks;
    while (current) {
        if (current->data == ptr) {
            current->is_free = 1;
            pool->used_size -= current->size;
            return;
        }
        current = current->next;
    }
}

size_t raf_pool_available(const raf_memory_pool *pool) {
    if (!pool) return 0;
    return pool->total_size - pool->used_size;
}

/* Ring Buffer Implementation */
raf_ring_buffer* raf_ring_create(size_t capacity) {
    raf_ring_buffer *ring = (raf_ring_buffer*)malloc(sizeof(raf_ring_buffer));
    if (!ring) return NULL;
    
    ring->buffer = (uint8_t*)malloc(capacity);
    if (!ring->buffer) {
        free(ring);
        return NULL;
    }
    
    ring->capacity = capacity;
    ring->head = 0;
    ring->tail = 0;
    ring->count = 0;
    
    return ring;
}

void raf_ring_destroy(raf_ring_buffer *ring) {
    if (ring) {
        if (ring->buffer) free(ring->buffer);
        free(ring);
    }
}

int raf_ring_push(raf_ring_buffer *ring, uint8_t value) {
    if (!ring || ring->count >= ring->capacity) return -1;
    
    ring->buffer[ring->tail] = value;
    ring->tail = (ring->tail + 1) % ring->capacity;
    ring->count++;
    
    return 0;
}

int raf_ring_pop(raf_ring_buffer *ring, uint8_t *value) {
    if (!ring || !value || ring->count == 0) return -1;
    
    *value = ring->buffer[ring->head];
    ring->head = (ring->head + 1) % ring->capacity;
    ring->count--;
    
    return 0;
}

size_t raf_ring_available(const raf_ring_buffer *ring) {
    if (!ring) return 0;
    return ring->capacity - ring->count;
}

int raf_ring_is_empty(const raf_ring_buffer *ring) {
    return ring ? (ring->count == 0) : 1;
}

int raf_ring_is_full(const raf_ring_buffer *ring) {
    return ring ? (ring->count >= ring->capacity) : 0;
}

/* Hash function (DJB2) */
uint32_t raf_hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash;
}

/* Key-Value Store Implementation */
raf_kv_store* raf_kv_create(size_t num_buckets) {
    if (num_buckets == 0) num_buckets = 256;
    
    raf_kv_store *store = (raf_kv_store*)malloc(sizeof(raf_kv_store));
    if (!store) return NULL;
    
    store->buckets = (raf_kv_entry**)calloc(num_buckets, sizeof(raf_kv_entry*));
    if (!store->buckets) {
        free(store);
        return NULL;
    }
    
    store->num_buckets = num_buckets;
    store->num_entries = 0;
    
    return store;
}

void raf_kv_destroy(raf_kv_store *store) {
    if (!store) return;
    
    for (size_t i = 0; i < store->num_buckets; i++) {
        raf_kv_entry *entry = store->buckets[i];
        while (entry) {
            raf_kv_entry *next = entry->next;
            if (entry->key) free(entry->key);
            if (entry->value) free(entry->value);
            free(entry);
            entry = next;
        }
    }
    
    free(store->buckets);
    free(store);
}

int raf_kv_set(raf_kv_store *store, const char *key, const void *value, size_t value_size) {
    if (!store || !key || !value) return -1;
    
    uint32_t hash = raf_hash_string(key);
    size_t bucket = hash % store->num_buckets;
    
    /* Check if key already exists */
    raf_kv_entry *entry = store->buckets[bucket];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* Update existing entry */
            void *new_value = malloc(value_size);
            if (!new_value) return -1;
            memcpy(new_value, value, value_size);
            free(entry->value);
            entry->value = new_value;
            entry->value_size = value_size;
            return 0;
        }
        entry = entry->next;
    }
    
    /* Create new entry */
    entry = (raf_kv_entry*)malloc(sizeof(raf_kv_entry));
    if (!entry) return -1;
    
    size_t key_len = strlen(key);
    entry->key = (char*)malloc(key_len + 1);
    if (!entry->key) {
        free(entry);
        return -1;
    }
    /* Safe to use strcpy since we allocated exact size needed */
    strcpy(entry->key, key);
    
    entry->value = malloc(value_size);
    if (!entry->value) {
        free(entry->key);
        free(entry);
        return -1;
    }
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    
    entry->next = store->buckets[bucket];
    store->buckets[bucket] = entry;
    store->num_entries++;
    
    return 0;
}

int raf_kv_get(raf_kv_store *store, const char *key, void **value, size_t *value_size) {
    if (!store || !key || !value) return -1;
    
    uint32_t hash = raf_hash_string(key);
    size_t bucket = hash % store->num_buckets;
    
    raf_kv_entry *entry = store->buckets[bucket];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            *value = entry->value;
            if (value_size) *value_size = entry->value_size;
            return 0;
        }
        entry = entry->next;
    }
    
    return -1; /* Not found */
}

int raf_kv_exists(raf_kv_store *store, const char *key) {
    void *value;
    return raf_kv_get(store, key, &value, NULL) == 0;
}

/* LRU Cache Implementation */
raf_lru_cache* raf_cache_create(size_t capacity) {
    raf_lru_cache *cache = (raf_lru_cache*)malloc(sizeof(raf_lru_cache));
    if (!cache) return NULL;
    
    cache->entries = NULL;
    cache->capacity = capacity;
    cache->count = 0;
    cache->access_counter = 0;
    
    return cache;
}

void raf_cache_destroy(raf_lru_cache *cache) {
    if (!cache) return;
    
    raf_cache_entry *entry = cache->entries;
    while (entry) {
        raf_cache_entry *next = entry->next;
        if (entry->key) free(entry->key);
        if (entry->data) free(entry->data);
        free(entry);
        entry = next;
    }
    
    free(cache);
}

void raf_cache_clear(raf_lru_cache *cache) {
    if (!cache) return;
    
    raf_cache_entry *entry = cache->entries;
    while (entry) {
        raf_cache_entry *next = entry->next;
        if (entry->key) free(entry->key);
        if (entry->data) free(entry->data);
        free(entry);
        entry = next;
    }
    
    cache->entries = NULL;
    cache->count = 0;
}
