/**
 * Rafaelia Baremetal - HD Cache MVP Implementation
 * 
 * Hierarchical disk cache with L1/L2/L3 tiers, TTL, retry logic,
 * and harmonic scheduling for event-driven processing.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_hd_cache.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

uint64_t raf_hd_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

uint64_t raf_hd_align_up(uint64_t value, uint64_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

/* Simplified SHA-256 (FNV-1a based hash for baremetal simplicity) */
void raf_hd_sha256(const uint8_t *data, uint32_t len, uint8_t hash[RAF_HD_HASH_SIZE]) {
    /* FNV-1a hash spread across 32 bytes for deterministic hashing */
    uint64_t h1 = 0xcbf29ce484222325ULL; /* FNV offset basis */
    uint64_t h2 = 0x100000001b3ULL;       /* FNV prime */
    uint64_t h3 = 0x9e3779b97f4a7c15ULL;
    uint64_t h4 = 0x517cc1b727220a95ULL;
    
    for (uint32_t i = 0; i < len; i++) {
        h1 ^= data[i];
        h1 *= 0x100000001b3ULL;
        h2 ^= data[i];
        h2 *= 0xcbf29ce484222325ULL;
        h3 = (h3 ^ data[i]) * 0x9e3779b97f4a7c15ULL;
        h4 = (h4 ^ data[i]) * 0x517cc1b727220a95ULL;
    }
    
    /* Spread hash values across 32 bytes */
    memcpy(hash + 0, &h1, 8);
    memcpy(hash + 8, &h2, 8);
    memcpy(hash + 16, &h3, 8);
    memcpy(hash + 24, &h4, 8);
}

int raf_hd_key_cmp(const raf_hd_event_key_t *k1, const raf_hd_event_key_t *k2) {
    int cmp = strncmp(k1->layer, k2->layer, sizeof(k1->layer));
    if (cmp != 0) return cmp;
    return strncmp(k1->eid, k2->eid, sizeof(k1->eid));
}

uint32_t raf_hd_key_hash(const raf_hd_event_key_t *key) {
    uint32_t hash = 5381;
    for (size_t i = 0; i < sizeof(key->layer) && key->layer[i]; i++) {
        hash = ((hash << 5) + hash) + (uint32_t)key->layer[i];
    }
    for (size_t i = 0; i < sizeof(key->eid) && key->eid[i]; i++) {
        hash = ((hash << 5) + hash) + (uint32_t)key->eid[i];
    }
    return hash;
}

/* ============================================================================
 * Block Store Implementation
 * ============================================================================ */

int raf_hd_store_init(raf_hd_block_store_t *store, 
                      const char *store_path,
                      const char *index_path) {
    if (!store || !store_path || !index_path) return -1;
    
    strncpy(store->store_path, store_path, sizeof(store->store_path) - 1);
    strncpy(store->index_path, index_path, sizeof(store->index_path) - 1);
    
    /* Open store file (create if doesn't exist) */
    store->fd = open(store_path, O_CREAT | O_RDWR, 0600);
    if (store->fd < 0) return -1;
    
    /* Get current size */
    store->size = lseek(store->fd, 0, SEEK_END);
    
    /* Ensure index exists */
    int idx_fd = open(index_path, O_CREAT | O_WRONLY | O_APPEND, 0600);
    if (idx_fd < 0) {
        close(store->fd);
        return -1;
    }
    close(idx_fd);
    
    return 0;
}

void raf_hd_store_close(raf_hd_block_store_t *store) {
    if (store && store->fd >= 0) {
        close(store->fd);
        store->fd = -1;
    }
}

int raf_hd_store_append(raf_hd_block_store_t *store,
                        const uint8_t *payload,
                        uint32_t payload_len,
                        uint64_t *offset,
                        uint32_t *disk_len,
                        uint8_t hash[RAF_HD_HASH_SIZE]) {
    if (!store || !payload || payload_len == 0 || payload_len > RAF_HD_MAX_BLOCK_BYTES) {
        return -1;
    }
    
    /* Compute hash */
    raf_hd_sha256(payload, payload_len, hash);
    
    /* Build block header:
     * [MAGIC 4B][VER 1B][FLAGS 1B][RESV 2B][PAYLEN 4B][HASH 32B][PAYLOAD][PADDING]
     */
    uint8_t header[48]; /* 4 + 1 + 1 + 2 + 4 + 32 + 4 (alignment) = 48 */
    uint32_t magic = RAF_HD_MAGIC;
    uint8_t version = RAF_HD_VERSION;
    uint8_t flags = 0;
    uint16_t resv = 0;
    
    memcpy(header + 0, &magic, 4);
    memcpy(header + 4, &version, 1);
    memcpy(header + 5, &flags, 1);
    memcpy(header + 6, &resv, 2);
    memcpy(header + 8, &payload_len, 4);
    memcpy(header + 12, hash, RAF_HD_HASH_SIZE);
    
    /* Calculate total size with padding */
    uint32_t header_size = 44; /* Actual header size */
    uint32_t total_size = header_size + payload_len;
    uint32_t aligned_size = (uint32_t)raf_hd_align_up(total_size, RAF_HD_BLOCK_ALIGN);
    uint32_t padding_size = aligned_size - total_size;
    
    *offset = store->size;
    *disk_len = aligned_size;
    
    /* Seek to end */
    if (lseek(store->fd, store->size, SEEK_SET) < 0) return -1;
    
    /* Write header */
    if (write(store->fd, header, header_size) != (ssize_t)header_size) return -1;
    
    /* Write payload */
    if (write(store->fd, payload, payload_len) != (ssize_t)payload_len) return -1;
    
    /* Write padding */
    if (padding_size > 0) {
        uint8_t *padding = calloc(1, padding_size);
        if (!padding) return -1;
        ssize_t written = write(store->fd, padding, padding_size);
        free(padding);
        if (written != (ssize_t)padding_size) return -1;
    }
    
    /* Sync to disk */
    fsync(store->fd);
    
    /* Update size */
    store->size += aligned_size;
    
    return 0;
}

int raf_hd_store_read(raf_hd_block_store_t *store,
                      uint64_t offset,
                      uint8_t **payload,
                      uint32_t *payload_len) {
    if (!store || !payload || !payload_len) return -1;
    
    /* Seek to offset */
    if (lseek(store->fd, offset, SEEK_SET) < 0) return -1;
    
    /* Read header */
    uint8_t header[44];
    if (read(store->fd, header, 44) != 44) return -1;
    
    /* Verify magic and version */
    uint32_t magic;
    uint8_t version;
    memcpy(&magic, header + 0, 4);
    memcpy(&version, header + 4, 1);
    
    if (magic != RAF_HD_MAGIC || version != RAF_HD_VERSION) return -1;
    
    /* Extract payload length */
    memcpy(payload_len, header + 8, 4);
    
    if (*payload_len == 0 || *payload_len > RAF_HD_MAX_BLOCK_BYTES) return -1;
    
    /* Extract hash */
    uint8_t stored_hash[RAF_HD_HASH_SIZE];
    memcpy(stored_hash, header + 12, RAF_HD_HASH_SIZE);
    
    /* Read payload */
    *payload = malloc(*payload_len);
    if (!*payload) return -1;
    
    if (read(store->fd, *payload, *payload_len) != (ssize_t)*payload_len) {
        free(*payload);
        *payload = NULL;
        return -1;
    }
    
    /* Verify hash */
    uint8_t computed_hash[RAF_HD_HASH_SIZE];
    raf_hd_sha256(*payload, *payload_len, computed_hash);
    
    if (memcmp(stored_hash, computed_hash, RAF_HD_HASH_SIZE) != 0) {
        free(*payload);
        *payload = NULL;
        return -1; /* Hash mismatch - corruption detected */
    }
    
    return 0;
}

int raf_hd_store_write_index(raf_hd_block_store_t *store,
                              const raf_hd_event_meta_t *meta) {
    if (!store || !meta) return -1;
    
    /* Open index file in append mode */
    FILE *f = fopen(store->index_path, "a");
    if (!f) return -1;
    
    /* Write JSON line (simplified format) */
    fprintf(f, "{\"layer\":\"%s\",\"eid\":\"%s\",\"created\":%llu,\"ttl\":%llu,"
            "\"retries\":%u,\"len\":%u,\"off\":%llu,\"dlen\":%u,\"status\":%d}\n",
            meta->key.layer, meta->key.eid, 
            (unsigned long long)meta->created_ns,
            (unsigned long long)meta->ttl_ns,
            meta->retries_left, meta->payload_len,
            (unsigned long long)meta->disk_off, meta->disk_len,
            (int)meta->status);
    
    fflush(f);
    int fd = fileno(f);
    if (fd >= 0) fsync(fd);
    fclose(f);
    
    return 0;
}

/* ============================================================================
 * Cache Tier Helper Functions
 * ============================================================================ */

static void tier_init(raf_hd_cache_tier_t *tier, uint64_t budget) {
    tier->budget = budget;
    tier->used = 0;
    tier->head = NULL;
    tier->tail = NULL;
    tier->count = 0;
}

static void tier_destroy(raf_hd_cache_tier_t *tier) {
    raf_hd_cache_entry_t *entry = tier->head;
    while (entry) {
        raf_hd_cache_entry_t *next = entry->next;
        if (entry->data) free(entry->data);
        free(entry);
        entry = next;
    }
    tier->head = NULL;
    tier->tail = NULL;
    tier->used = 0;
    tier->count = 0;
}

static raf_hd_cache_entry_t* tier_find(raf_hd_cache_tier_t *tier, 
                                        const raf_hd_event_key_t *key) {
    raf_hd_cache_entry_t *entry = tier->head;
    while (entry) {
        if (raf_hd_key_cmp(&entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static void tier_remove(raf_hd_cache_tier_t *tier, raf_hd_cache_entry_t *entry) {
    if (entry->prev) entry->prev->next = entry->next;
    else tier->head = entry->next;
    
    if (entry->next) entry->next->prev = entry->prev;
    else tier->tail = entry->prev;
    
    tier->used -= entry->data_len;
    tier->count--;
}

static void tier_add_head(raf_hd_cache_tier_t *tier, raf_hd_cache_entry_t *entry) {
    entry->next = tier->head;
    entry->prev = NULL;
    
    if (tier->head) tier->head->prev = entry;
    tier->head = entry;
    
    if (!tier->tail) tier->tail = entry;
    
    tier->used += entry->data_len;
    tier->count++;
}

static int tier_put(raf_hd_cache_tier_t *tier, 
                    const raf_hd_event_key_t *key,
                    const uint8_t *data, 
                    uint32_t data_len) {
    /* Check if already exists */
    raf_hd_cache_entry_t *existing = tier_find(tier, key);
    if (existing) {
        /* Move to head (most recent) */
        tier_remove(tier, existing);
        tier_add_head(tier, existing);
        return 0;
    }
    
    /* Create new entry */
    raf_hd_cache_entry_t *entry = malloc(sizeof(raf_hd_cache_entry_t));
    if (!entry) return -1;
    
    entry->data = malloc(data_len);
    if (!entry->data) {
        free(entry);
        return -1;
    }
    
    memcpy(&entry->key, key, sizeof(raf_hd_event_key_t));
    memcpy(entry->data, data, data_len);
    entry->data_len = data_len;
    
    tier_add_head(tier, entry);
    
    return 0;
}

static raf_hd_cache_entry_t* tier_pop_tail(raf_hd_cache_tier_t *tier) {
    if (!tier->tail) return NULL;
    
    raf_hd_cache_entry_t *entry = tier->tail;
    tier_remove(tier, entry);
    
    return entry;
}

/* ============================================================================
 * Cache Implementation
 * ============================================================================ */

int raf_hd_cache_init(raf_hd_l123_cache_t *cache) {
    if (!cache) return -1;
    
    tier_init(&cache->l1, RAF_HD_L1_BUDGET);
    tier_init(&cache->l2, RAF_HD_L2_BUDGET);
    tier_init(&cache->l3, RAF_HD_L3_BUDGET);
    
    return 0;
}

void raf_hd_cache_destroy(raf_hd_l123_cache_t *cache) {
    if (!cache) return;
    
    tier_destroy(&cache->l1);
    tier_destroy(&cache->l2);
    tier_destroy(&cache->l3);
}

int raf_hd_cache_put_hot(raf_hd_l123_cache_t *cache,
                         const raf_hd_event_key_t *key,
                         const uint8_t *data,
                         uint32_t data_len) {
    if (!cache || !key || !data || data_len == 0) return -1;
    
    return tier_put(&cache->l1, key, data, data_len);
}

int raf_hd_cache_get(raf_hd_l123_cache_t *cache,
                     const raf_hd_event_key_t *key,
                     uint8_t **data,
                     uint32_t *data_len) {
    if (!cache || !key || !data || !data_len) return -1;
    
    /* Try L1 */
    raf_hd_cache_entry_t *entry = tier_find(&cache->l1, key);
    if (entry) {
        *data = entry->data;
        *data_len = entry->data_len;
        return 0;
    }
    
    /* Try L2 */
    entry = tier_find(&cache->l2, key);
    if (entry) {
        *data = entry->data;
        *data_len = entry->data_len;
        return 0;
    }
    
    /* Try L3 */
    entry = tier_find(&cache->l3, key);
    if (entry) {
        *data = entry->data;
        *data_len = entry->data_len;
        return 0;
    }
    
    return -1; /* Not found */
}

void raf_hd_cache_demote_cycle(raf_hd_l123_cache_t *cache) {
    if (!cache) return;
    
    /* Demote from L1 to L2 if over budget */
    while (cache->l1.used > cache->l1.budget) {
        raf_hd_cache_entry_t *entry = tier_pop_tail(&cache->l1);
        if (!entry) break;
        
        tier_put(&cache->l2, &entry->key, entry->data, entry->data_len);
        
        if (entry->data) free(entry->data);
        free(entry);
    }
    
    /* Demote from L2 to L3 if over budget */
    while (cache->l2.used > cache->l2.budget) {
        raf_hd_cache_entry_t *entry = tier_pop_tail(&cache->l2);
        if (!entry) break;
        
        tier_put(&cache->l3, &entry->key, entry->data, entry->data_len);
        
        if (entry->data) free(entry->data);
        free(entry);
    }
    
    /* Evict from L3 if over budget */
    while (cache->l3.used > cache->l3.budget) {
        raf_hd_cache_entry_t *entry = tier_pop_tail(&cache->l3);
        if (!entry) break;
        
        if (entry->data) free(entry->data);
        free(entry);
    }
}

/* ============================================================================
 * Scheduler Implementation
 * ============================================================================ */

int raf_hd_scheduler_init(raf_hd_scheduler_t *scheduler) {
    if (!scheduler) return -1;
    
    scheduler->tick = 0;
    scheduler->num_layers = 0;
    scheduler->capacity = 16;
    scheduler->layers = calloc(scheduler->capacity, sizeof(raf_hd_layer_t));
    
    if (!scheduler->layers) return -1;
    
    return 0;
}

void raf_hd_scheduler_destroy(raf_hd_scheduler_t *scheduler) {
    if (!scheduler) return;
    
    for (uint32_t i = 0; i < scheduler->num_layers; i++) {
        /* Free queue entries */
        raf_hd_queue_entry_t *entry = scheduler->layers[i].queue.head;
        while (entry) {
            raf_hd_queue_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    
    if (scheduler->layers) free(scheduler->layers);
    scheduler->layers = NULL;
}

int raf_hd_scheduler_add_layer(raf_hd_scheduler_t *scheduler,
                                const char *name,
                                uint32_t frequency) {
    if (!scheduler || !name) return -1;
    
    /* Grow capacity if needed */
    if (scheduler->num_layers >= scheduler->capacity) {
        uint32_t new_capacity = scheduler->capacity * 2;
        raf_hd_layer_t *new_layers = realloc(scheduler->layers, 
                                              new_capacity * sizeof(raf_hd_layer_t));
        if (!new_layers) return -1;
        
        scheduler->layers = new_layers;
        scheduler->capacity = new_capacity;
    }
    
    /* Initialize layer */
    raf_hd_layer_t *layer = &scheduler->layers[scheduler->num_layers];
    strncpy(layer->name, name, sizeof(layer->name) - 1);
    layer->name[sizeof(layer->name) - 1] = '\0';
    layer->frequency = frequency;
    layer->queue.head = NULL;
    layer->queue.tail = NULL;
    layer->queue.count = 0;
    
    scheduler->num_layers++;
    
    return 0;
}

static uint32_t scheduler_weight(uint32_t freq) {
    if (freq >= 100000) return 64;
    if (freq >= 1000) return 16;
    if (freq >= 500) return 8;
    if (freq >= 100) return 4;
    return 2;
}

int raf_hd_scheduler_next(raf_hd_scheduler_t *scheduler) {
    if (!scheduler || scheduler->num_layers == 0) return -1;
    
    /* Increment tick */
    scheduler->tick++;
    
    /* Compute weighted schedule based on harmonic frequencies */
    /* Use tick counter for deterministic rotation */
    uint32_t total_weight = 0;
    uint32_t weights[64]; /* Max 64 layers */
    
    for (uint32_t i = 0; i < scheduler->num_layers && i < 64; i++) {
        weights[i] = scheduler_weight(scheduler->layers[i].frequency);
        total_weight += weights[i];
    }
    
    if (total_weight == 0) return -1;
    
    /* Use tick modulo total weight to select layer */
    uint32_t target = (uint32_t)(scheduler->tick % total_weight);
    uint32_t cumulative = 0;
    
    for (uint32_t i = 0; i < scheduler->num_layers; i++) {
        cumulative += weights[i];
        if (target < cumulative) {
            /* Check if this layer has queued events */
            if (scheduler->layers[i].queue.count > 0) {
                return (int)i;
            }
        }
    }
    
    /* Fallback: find any layer with events */
    for (uint32_t i = 0; i < scheduler->num_layers; i++) {
        if (scheduler->layers[i].queue.count > 0) {
            return (int)i;
        }
    }
    
    return -1; /* No events in any queue */
}

/* ============================================================================
 * Metadata Store Implementation
 * ============================================================================ */

int raf_hd_meta_init(raf_hd_meta_store_t *meta) {
    if (!meta) return -1;
    
    meta->buckets = calloc(RAF_HD_META_BUCKETS, sizeof(raf_hd_event_meta_t*));
    if (!meta->buckets) return -1;
    
    meta->count = 0;
    
    return 0;
}

void raf_hd_meta_destroy(raf_hd_meta_store_t *meta) {
    if (!meta || !meta->buckets) return;
    
    for (uint32_t i = 0; i < RAF_HD_META_BUCKETS; i++) {
        raf_hd_event_meta_t *entry = meta->buckets[i];
        while (entry) {
            raf_hd_event_meta_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    
    free(meta->buckets);
    meta->buckets = NULL;
}

int raf_hd_meta_put(raf_hd_meta_store_t *meta, const raf_hd_event_meta_t *event) {
    if (!meta || !event) return -1;
    
    uint32_t hash = raf_hd_key_hash(&event->key);
    uint32_t bucket = hash % RAF_HD_META_BUCKETS;
    
    raf_hd_event_meta_t *entry = meta->buckets[bucket];
    while (entry) {
        if (raf_hd_key_cmp(&entry->key, &event->key) == 0) {
            raf_hd_event_meta_t *next = entry->next;
            memcpy(entry, event, sizeof(raf_hd_event_meta_t));
            entry->next = next;
            return 0;
        }
        entry = entry->next;
    }
    
    /* Allocate new entry */
    entry = malloc(sizeof(raf_hd_event_meta_t));
    if (!entry) return -1;
    
    memcpy(entry, event, sizeof(raf_hd_event_meta_t));
    entry->next = meta->buckets[bucket];
    meta->buckets[bucket] = entry;
    meta->count++;
    
    return 0;
}

raf_hd_event_meta_t* raf_hd_meta_get(raf_hd_meta_store_t *meta, 
                                     const raf_hd_event_key_t *key) {
    if (!meta || !key) return NULL;
    
    uint32_t hash = raf_hd_key_hash(key);
    uint32_t bucket = hash % RAF_HD_META_BUCKETS;
    
    raf_hd_event_meta_t *entry = meta->buckets[bucket];
    while (entry) {
        if (raf_hd_key_cmp(&entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    
    return NULL;
}

/* ============================================================================
 * HD Cache Engine Implementation
 * ============================================================================ */

raf_hd_cache_t* raf_hd_cache_create(const char *store_path, const char *index_path) {
    if (!store_path || !index_path) return NULL;
    
    raf_hd_cache_t *cache = calloc(1, sizeof(raf_hd_cache_t));
    if (!cache) return NULL;
    
    /* Initialize block store */
    if (raf_hd_store_init(&cache->store, store_path, index_path) != 0) {
        free(cache);
        return NULL;
    }
    
    /* Initialize cache */
    if (raf_hd_cache_init(&cache->cache) != 0) {
        raf_hd_store_close(&cache->store);
        free(cache);
        return NULL;
    }
    
    /* Initialize scheduler */
    if (raf_hd_scheduler_init(&cache->scheduler) != 0) {
        raf_hd_cache_destroy(&cache->cache);
        raf_hd_store_close(&cache->store);
        free(cache);
        return NULL;
    }
    
    /* Initialize metadata store */
    if (raf_hd_meta_init(&cache->meta) != 0) {
        raf_hd_scheduler_destroy(&cache->scheduler);
        raf_hd_cache_destroy(&cache->cache);
        raf_hd_store_close(&cache->store);
        free(cache);
        return NULL;
    }
    
    cache->total_events = 0;
    cache->dropped_events = 0;
    
    return cache;
}

void raf_hd_cache_destroy_engine(raf_hd_cache_t *cache) {
    if (!cache) return;
    
    raf_hd_meta_destroy(&cache->meta);
    raf_hd_scheduler_destroy(&cache->scheduler);
    raf_hd_cache_destroy(&cache->cache);
    raf_hd_store_close(&cache->store);
    
    free(cache);
}

int raf_hd_cache_add_layer(raf_hd_cache_t *cache, const char *layer, uint32_t frequency) {
    if (!cache || !layer) return -1;
    
    return raf_hd_scheduler_add_layer(&cache->scheduler, layer, frequency);
}

int raf_hd_cache_ingest(raf_hd_cache_t *cache,
                        const char *layer,
                        const uint8_t *payload,
                        uint32_t payload_len,
                        uint64_t ttl_ns,
                        raf_hd_event_key_t *key) {
    if (!cache || !layer || !payload || payload_len == 0 || !key) return -1;
    
    /* Find layer */
    int layer_idx = -1;
    for (uint32_t i = 0; i < cache->scheduler.num_layers; i++) {
        if (strcmp(cache->scheduler.layers[i].name, layer) == 0) {
            layer_idx = (int)i;
            break;
        }
    }
    
    if (layer_idx < 0) return -1;
    
    /* Generate event ID */
    uint64_t now = raf_hd_now_ns();
    snprintf(key->eid, sizeof(key->eid), "%llx-%08x", 
             (unsigned long long)now, rand());
    strncpy(key->layer, layer, sizeof(key->layer) - 1);
    key->layer[sizeof(key->layer) - 1] = '\0';
    
    /* Append to store */
    uint64_t offset;
    uint32_t disk_len;
    uint8_t hash[RAF_HD_HASH_SIZE];
    
    if (raf_hd_store_append(&cache->store, payload, payload_len, 
                            &offset, &disk_len, hash) != 0) {
        return -1;
    }
    
    /* Create metadata */
    raf_hd_event_meta_t meta;
    memcpy(&meta.key, key, sizeof(raf_hd_event_key_t));
    meta.created_ns = now;
    meta.ttl_ns = ttl_ns > 0 ? ttl_ns : RAF_HD_DEFAULT_TTL_NS;
    meta.retries_left = RAF_HD_MAX_RETRIES;
    meta.payload_len = payload_len;
    memcpy(meta.payload_hash, hash, RAF_HD_HASH_SIZE);
    meta.disk_off = offset;
    meta.disk_len = disk_len;
    meta.status = RAF_HD_STATUS_NEW;
    
    /* Store metadata */
    raf_hd_meta_put(&cache->meta, &meta);
    raf_hd_store_write_index(&cache->store, &meta);
    
    /* Put in L1 cache */
    raf_hd_cache_put_hot(&cache->cache, key, payload, payload_len);
    
    /* Enqueue event */
    raf_hd_queue_entry_t *qentry = malloc(sizeof(raf_hd_queue_entry_t));
    if (!qentry) return -1;
    
    memcpy(&qentry->key, key, sizeof(raf_hd_event_key_t));
    qentry->next = NULL;
    
    raf_hd_event_queue_t *queue = &cache->scheduler.layers[layer_idx].queue;
    if (queue->tail) {
        queue->tail->next = qentry;
    } else {
        queue->head = qentry;
    }
    queue->tail = qentry;
    queue->count++;
    
    cache->total_events++;
    
    /* Check for queue overflow and drop oldest if needed */
    uint32_t total_queued = 0;
    for (uint32_t i = 0; i < cache->scheduler.num_layers; i++) {
        total_queued += cache->scheduler.layers[i].queue.count;
    }
    
    if (total_queued > RAF_HD_DROP_QUEUE_SIZE) {
        /* Drop oldest from largest queue */
        uint32_t max_count = 0;
        int max_idx = -1;
        
        for (uint32_t i = 0; i < cache->scheduler.num_layers; i++) {
            if (cache->scheduler.layers[i].queue.count > max_count) {
                max_count = cache->scheduler.layers[i].queue.count;
                max_idx = (int)i;
            }
        }
        
        if (max_idx >= 0) {
            raf_hd_event_queue_t *drop_queue = &cache->scheduler.layers[max_idx].queue;
            raf_hd_queue_entry_t *drop_entry = drop_queue->head;
            
            if (drop_entry) {
                drop_queue->head = drop_entry->next;
                if (!drop_queue->head) drop_queue->tail = NULL;
                drop_queue->count--;
                
                /* Update metadata */
                raf_hd_event_meta_t *drop_meta = raf_hd_meta_get(&cache->meta, &drop_entry->key);
                if (drop_meta) {
                    drop_meta->status = RAF_HD_STATUS_DROPPED;
                    raf_hd_store_write_index(&cache->store, drop_meta);
                }
                
                free(drop_entry);
                cache->dropped_events++;
            }
        }
    }
    
    return 0;
}

uint32_t raf_hd_cache_tick(raf_hd_cache_t *cache,
                           uint32_t max_events,
                           raf_hd_process_fn process_fn,
                           void *user_data) {
    if (!cache || !process_fn) return 0;
    
    uint32_t processed = 0;
    uint64_t now = raf_hd_now_ns();
    
    for (uint32_t i = 0; i < max_events; i++) {
        /* Get next layer to process */
        int layer_idx = raf_hd_scheduler_next(&cache->scheduler);
        if (layer_idx < 0) break; /* No more events */
        
        /* Dequeue event */
        raf_hd_event_queue_t *queue = &cache->scheduler.layers[layer_idx].queue;
        raf_hd_queue_entry_t *qentry = queue->head;
        
        if (!qentry) continue;
        
        queue->head = qentry->next;
        if (!queue->head) queue->tail = NULL;
        queue->count--;
        
        /* Get metadata */
        raf_hd_event_meta_t *meta = raf_hd_meta_get(&cache->meta, &qentry->key);
        if (!meta) {
            free(qentry);
            continue;
        }
        
        /* Check if expired */
        if (meta->status != RAF_HD_STATUS_DROPPED && 
            meta->status != RAF_HD_STATUS_DONE &&
            (now - meta->created_ns) > meta->ttl_ns) {
            meta->status = RAF_HD_STATUS_EXPIRED;
            raf_hd_store_write_index(&cache->store, meta);
            free(qentry);
            processed++;
            continue;
        }
        
        /* Skip if already done or dropped */
        if (meta->status == RAF_HD_STATUS_DONE || 
            meta->status == RAF_HD_STATUS_DROPPED ||
            meta->status == RAF_HD_STATUS_EXPIRED) {
            free(qentry);
            continue;
        }
        
        /* Fetch payload */
        uint8_t *payload = NULL;
        uint32_t payload_len = 0;
        int from_disk = 0; /* Track if we allocated from disk */
        int requeued = 0;
        
        /* Try cache first */
        if (raf_hd_cache_get(&cache->cache, &qentry->key, &payload, &payload_len) != 0) {
            /* Read from disk */
            if (raf_hd_store_read(&cache->store, meta->disk_off, &payload, &payload_len) != 0) {
                /* Read failed - retry or drop */
                if (meta->retries_left > 0) {
                    meta->retries_left--;
                    meta->status = RAF_HD_STATUS_RETRYING;
                    raf_hd_store_write_index(&cache->store, meta);
                    
                    /* Requeue at end */
                    qentry->next = NULL;
                    if (queue->tail) {
                        queue->tail->next = qentry;
                    } else {
                        queue->head = qentry;
                    }
                    queue->tail = qentry;
                    queue->count++;
                    requeued = 1;
                } else {
                    meta->status = RAF_HD_STATUS_DROPPED;
                    raf_hd_store_write_index(&cache->store, meta);
                    free(qentry);
                }
                
                processed++;
                continue;
            }
            from_disk = 1; /* We allocated from disk */
        }
        
        /* Process event */
        int result = process_fn(&qentry->key, payload, payload_len, user_data);
        
        /* Free payload if we allocated it from disk */
        if (from_disk && payload) {
            free(payload);
        }
        
        if (result == 0) {
            /* Success */
            meta->status = RAF_HD_STATUS_DONE;
            raf_hd_store_write_index(&cache->store, meta);
        } else {
            /* Error - retry or drop */
            if (meta->retries_left > 0) {
                meta->retries_left--;
                meta->status = RAF_HD_STATUS_RETRYING;
                raf_hd_store_write_index(&cache->store, meta);
                
                /* Requeue */
                if (queue->tail) {
                    queue->tail->next = qentry;
                } else {
                    queue->head = qentry;
                }
                queue->tail = qentry;
                qentry->next = NULL;
                queue->count++;
                requeued = 1;
            } else {
                meta->status = RAF_HD_STATUS_DROPPED;
                raf_hd_store_write_index(&cache->store, meta);
            }
        }
        
        if (!requeued) {
            free(qentry);
        }
        processed++;
    }
    
    /* Demote cache tiers */
    raf_hd_cache_demote_cycle(&cache->cache);
    
    return processed;
}

void raf_hd_cache_stats(raf_hd_cache_t *cache,
                        uint64_t *l1_used,
                        uint64_t *l2_used,
                        uint64_t *l3_used,
                        uint32_t *queue_size) {
    if (!cache) return;
    
    if (l1_used) *l1_used = cache->cache.l1.used;
    if (l2_used) *l2_used = cache->cache.l2.used;
    if (l3_used) *l3_used = cache->cache.l3.used;
    
    if (queue_size) {
        uint32_t total = 0;
        for (uint32_t i = 0; i < cache->scheduler.num_layers; i++) {
            total += cache->scheduler.layers[i].queue.count;
        }
        *queue_size = total;
    }
}
