/**
 * Rafaelia Baremetal - HD Cache MVP
 * 
 * Hierarchical disk cache with L1/L2/L3 tiers, TTL, retry logic,
 * and harmonic scheduling for event-driven processing.
 * 
 * Architecture:
 * - Append-only block store (page-aligned, checksummed)
 * - 3-tier cache (L1: hot, L2: warm, L3: cold)
 * - TTL-based expiration
 * - Retry logic with configurable limits
 * - Queue management with drop control
 * - Harmonic scheduler for deterministic processing
 * 
 * Design Philosophy:
 * - Baremetal: No external dependencies
 * - Deterministic: Reproducible behavior
 * - Hardware-aware: Page alignment, cache-friendly
 * - Zero-copy where possible
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_HD_CACHE_H
#define RAFAELIA_HD_CACHE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration Constants */
#define RAF_HD_BLOCK_ALIGN       4096              /* Page alignment (4KB) */
#define RAF_HD_MAX_BLOCK_BYTES   (256 * 1024)      /* Max payload per block (256KB) */
#define RAF_HD_DEFAULT_TTL_NS    (30LL * 1000000000LL) /* 30 seconds in nanoseconds */
#define RAF_HD_MAX_RETRIES       3                 /* Max retry attempts */
#define RAF_HD_DROP_QUEUE_SIZE   10000             /* Drop if queue exceeds this */
#define RAF_HD_MAGIC             0x52424630         /* "RBF0" magic number */
#define RAF_HD_VERSION           1                 /* Block format version */
#define RAF_HD_HASH_SIZE         32                /* SHA-256 hash size */

/* Cache tier budgets (bytes) */
#define RAF_HD_L1_BUDGET         (2 * 1024 * 1024)    /* 2MB - hottest */
#define RAF_HD_L2_BUDGET         (16 * 1024 * 1024)   /* 16MB - warm */
#define RAF_HD_L3_BUDGET         (128 * 1024 * 1024)  /* 128MB - cold */

/* Harmonic frequencies (symbolic weights) */
#define RAF_HD_NUM_HARMONICS     8
static const uint32_t RAF_HD_HARMONICS[RAF_HD_NUM_HARMONICS] = {
    12, 144, 288, 144000, 777, 555, 963, 999
};

/* Event Status */
typedef enum {
    RAF_HD_STATUS_NEW       = 0,
    RAF_HD_STATUS_HOT       = 1,
    RAF_HD_STATUS_COLD      = 2,
    RAF_HD_STATUS_EXPIRED   = 3,
    RAF_HD_STATUS_DROPPED   = 4,
    RAF_HD_STATUS_DONE      = 5,
    RAF_HD_STATUS_RETRYING  = 6,
} raf_hd_status_t;

/* Event Key - Unique identifier for an event */
typedef struct {
    char layer[32];      /* Layer name (e.g., "past", "present", "future") */
    char eid[64];        /* Event ID (unique) */
} raf_hd_event_key_t;

/* Event Metadata */
typedef struct raf_hd_event_meta {
    raf_hd_event_key_t key;
    uint64_t created_ns;        /* Creation timestamp (nanoseconds) */
    uint64_t ttl_ns;            /* Time to live (nanoseconds) */
    uint32_t retries_left;      /* Remaining retry attempts */
    uint32_t payload_len;       /* Payload size in bytes */
    uint8_t payload_hash[RAF_HD_HASH_SIZE]; /* SHA-256 hash of payload */
    uint64_t disk_off;          /* Offset in store file */
    uint32_t disk_len;          /* Length in store file (including padding) */
    raf_hd_status_t status;     /* Current status */
    struct raf_hd_event_meta *next; /* Next entry in hash bucket */
} raf_hd_event_meta_t;

/* Block Store - Append-only storage with alignment */
typedef struct {
    int fd;                     /* File descriptor */
    char store_path[256];       /* Path to store file */
    char index_path[256];       /* Path to index file */
    uint64_t size;              /* Current file size */
} raf_hd_block_store_t;

/* Cache Entry - For L1/L2/L3 tiers */
typedef struct raf_hd_cache_entry {
    raf_hd_event_key_t key;
    uint8_t *data;              /* Cached payload */
    uint32_t data_len;          /* Payload length */
    struct raf_hd_cache_entry *next;
    struct raf_hd_cache_entry *prev;
} raf_hd_cache_entry_t;

/* Cache Tier - Single tier (L1, L2, or L3) */
typedef struct {
    uint64_t budget;            /* Byte budget for this tier */
    uint64_t used;              /* Currently used bytes */
    raf_hd_cache_entry_t *head; /* Head of LRU list (most recent) */
    raf_hd_cache_entry_t *tail; /* Tail of LRU list (least recent) */
    uint32_t count;             /* Number of entries */
} raf_hd_cache_tier_t;

/* L1/L2/L3 Cache */
typedef struct {
    raf_hd_cache_tier_t l1;     /* Hottest (2MB) */
    raf_hd_cache_tier_t l2;     /* Warm (16MB) */
    raf_hd_cache_tier_t l3;     /* Cold (128MB) */
} raf_hd_l123_cache_t;

/* Event Queue Entry */
typedef struct raf_hd_queue_entry {
    raf_hd_event_key_t key;
    struct raf_hd_queue_entry *next;
} raf_hd_queue_entry_t;

/* Event Queue - Per-layer queue */
typedef struct {
    raf_hd_queue_entry_t *head;
    raf_hd_queue_entry_t *tail;
    uint32_t count;
} raf_hd_event_queue_t;

/* Layer Configuration */
typedef struct {
    char name[32];              /* Layer name */
    uint32_t frequency;         /* Harmonic frequency (scheduling weight) */
    raf_hd_event_queue_t queue; /* Event queue for this layer */
} raf_hd_layer_t;

/* Harmonic Scheduler */
typedef struct {
    uint64_t tick;              /* Current tick counter */
    uint32_t num_layers;        /* Number of registered layers */
    raf_hd_layer_t *layers;     /* Array of layers */
    uint32_t capacity;          /* Capacity of layers array */
} raf_hd_scheduler_t;

/* Event Metadata Store - Hash table for fast lookup */
#define RAF_HD_META_BUCKETS 1024
typedef struct {
    raf_hd_event_meta_t **buckets; /* Hash table buckets */
    uint32_t count;                 /* Total number of entries */
} raf_hd_meta_store_t;

/* HD Cache Engine - Main structure */
typedef struct {
    raf_hd_block_store_t store;     /* Append-only block store */
    raf_hd_l123_cache_t cache;      /* L1/L2/L3 cache */
    raf_hd_scheduler_t scheduler;   /* Harmonic scheduler */
    raf_hd_meta_store_t meta;       /* Event metadata store */
    uint64_t total_events;          /* Total events ingested */
    uint64_t dropped_events;        /* Events dropped due to queue overflow */
} raf_hd_cache_t;

/* Event processing callback - Returns 0 on success, -1 on error */
typedef int (*raf_hd_process_fn)(const raf_hd_event_key_t *key, 
                                  const uint8_t *payload, 
                                  uint32_t payload_len,
                                  void *user_data);

/* ============================================================================
 * Block Store API
 * ============================================================================ */

/**
 * Initialize block store
 * @param store Pointer to store structure
 * @param store_path Path to store file
 * @param index_path Path to index file
 * @return 0 on success, -1 on error
 */
int raf_hd_store_init(raf_hd_block_store_t *store, 
                      const char *store_path,
                      const char *index_path);

/**
 * Close block store
 * @param store Pointer to store structure
 */
void raf_hd_store_close(raf_hd_block_store_t *store);

/**
 * Append block to store
 * @param store Pointer to store structure
 * @param payload Payload data
 * @param payload_len Payload length
 * @param offset Output: offset in file
 * @param disk_len Output: total length in file (including padding)
 * @param hash Output: SHA-256 hash of payload
 * @return 0 on success, -1 on error
 */
int raf_hd_store_append(raf_hd_block_store_t *store,
                        const uint8_t *payload,
                        uint32_t payload_len,
                        uint64_t *offset,
                        uint32_t *disk_len,
                        uint8_t hash[RAF_HD_HASH_SIZE]);

/**
 * Read block from store
 * @param store Pointer to store structure
 * @param offset Offset in file
 * @param payload Output: payload data (caller must free)
 * @param payload_len Output: payload length
 * @return 0 on success, -1 on error
 */
int raf_hd_store_read(raf_hd_block_store_t *store,
                      uint64_t offset,
                      uint8_t **payload,
                      uint32_t *payload_len);

/**
 * Write event metadata to index
 * @param store Pointer to store structure
 * @param meta Event metadata
 * @return 0 on success, -1 on error
 */
int raf_hd_store_write_index(raf_hd_block_store_t *store,
                              const raf_hd_event_meta_t *meta);

/* ============================================================================
 * Cache API
 * ============================================================================ */

/**
 * Initialize L1/L2/L3 cache
 * @param cache Pointer to cache structure
 * @return 0 on success, -1 on error
 */
int raf_hd_cache_init(raf_hd_l123_cache_t *cache);

/**
 * Destroy cache and free all entries
 * @param cache Pointer to cache structure
 */
void raf_hd_cache_destroy(raf_hd_l123_cache_t *cache);

/**
 * Put entry in L1 (hot) cache
 * @param cache Pointer to cache structure
 * @param key Event key
 * @param data Payload data (will be copied)
 * @param data_len Payload length
 * @return 0 on success, -1 on error
 */
int raf_hd_cache_put_hot(raf_hd_l123_cache_t *cache,
                         const raf_hd_event_key_t *key,
                         const uint8_t *data,
                         uint32_t data_len);

/**
 * Get entry from cache (L1 -> L2 -> L3)
 * @param cache Pointer to cache structure
 * @param key Event key
 * @param data Output: payload data (caller must NOT free, points to cache)
 * @param data_len Output: payload length
 * @return 0 if found, -1 if not found
 */
int raf_hd_cache_get(raf_hd_l123_cache_t *cache,
                     const raf_hd_event_key_t *key,
                     uint8_t **data,
                     uint32_t *data_len);

/**
 * Demote entries from L1 -> L2 -> L3 when over budget
 * @param cache Pointer to cache structure
 */
void raf_hd_cache_demote_cycle(raf_hd_l123_cache_t *cache);

/* ============================================================================
 * Scheduler API
 * ============================================================================ */

/**
 * Initialize harmonic scheduler
 * @param scheduler Pointer to scheduler structure
 * @return 0 on success, -1 on error
 */
int raf_hd_scheduler_init(raf_hd_scheduler_t *scheduler);

/**
 * Destroy scheduler
 * @param scheduler Pointer to scheduler structure
 */
void raf_hd_scheduler_destroy(raf_hd_scheduler_t *scheduler);

/**
 * Add layer to scheduler
 * @param scheduler Pointer to scheduler structure
 * @param name Layer name
 * @param frequency Harmonic frequency (scheduling weight)
 * @return 0 on success, -1 on error
 */
int raf_hd_scheduler_add_layer(raf_hd_scheduler_t *scheduler,
                                const char *name,
                                uint32_t frequency);

/**
 * Get next layer to process in this cycle
 * @param scheduler Pointer to scheduler structure
 * @return Layer index, or -1 if none available
 */
int raf_hd_scheduler_next(raf_hd_scheduler_t *scheduler);

/* ============================================================================
 * Metadata Store API
 * ============================================================================ */

/**
 * Initialize metadata store
 * @param meta Pointer to metadata store structure
 * @return 0 on success, -1 on error
 */
int raf_hd_meta_init(raf_hd_meta_store_t *meta);

/**
 * Destroy metadata store
 * @param meta Pointer to metadata store structure
 */
void raf_hd_meta_destroy(raf_hd_meta_store_t *meta);

/**
 * Put event metadata
 * @param meta Pointer to metadata store structure
 * @param event Event metadata
 * @return 0 on success, -1 on error
 */
int raf_hd_meta_put(raf_hd_meta_store_t *meta, const raf_hd_event_meta_t *event);

/**
 * Get event metadata
 * @param meta Pointer to metadata store structure
 * @param key Event key
 * @return Pointer to metadata, or NULL if not found
 */
raf_hd_event_meta_t* raf_hd_meta_get(raf_hd_meta_store_t *meta, 
                                     const raf_hd_event_key_t *key);

/* ============================================================================
 * HD Cache Engine API
 * ============================================================================ */

/**
 * Create and initialize HD cache engine
 * @param store_path Path to store file
 * @param index_path Path to index file
 * @return Pointer to cache engine, or NULL on error
 */
raf_hd_cache_t* raf_hd_cache_create(const char *store_path, const char *index_path);

/**
 * Destroy HD cache engine
 * @param cache Pointer to cache engine
 */
void raf_hd_cache_destroy_engine(raf_hd_cache_t *cache);

/**
 * Add layer to cache engine
 * @param cache Pointer to cache engine
 * @param layer Layer name
 * @param frequency Harmonic frequency
 * @return 0 on success, -1 on error
 */
int raf_hd_cache_add_layer(raf_hd_cache_t *cache, const char *layer, uint32_t frequency);

/**
 * Ingest event into cache
 * @param cache Pointer to cache engine
 * @param layer Layer name
 * @param payload Payload data
 * @param payload_len Payload length
 * @param ttl_ns Time to live in nanoseconds (0 for default)
 * @param key Output: event key
 * @return 0 on success, -1 on error
 */
int raf_hd_cache_ingest(raf_hd_cache_t *cache,
                        const char *layer,
                        const uint8_t *payload,
                        uint32_t payload_len,
                        uint64_t ttl_ns,
                        raf_hd_event_key_t *key);

/**
 * Process events for one tick
 * @param cache Pointer to cache engine
 * @param max_events Maximum events to process this tick
 * @param process_fn Callback function to process each event
 * @param user_data User data passed to callback
 * @return Number of events processed
 */
uint32_t raf_hd_cache_tick(raf_hd_cache_t *cache,
                           uint32_t max_events,
                           raf_hd_process_fn process_fn,
                           void *user_data);

/**
 * Get cache statistics
 * @param cache Pointer to cache engine
 * @param l1_used Output: L1 cache used bytes
 * @param l2_used Output: L2 cache used bytes
 * @param l3_used Output: L3 cache used bytes
 * @param queue_size Output: total queue size across all layers
 */
void raf_hd_cache_stats(raf_hd_cache_t *cache,
                        uint64_t *l1_used,
                        uint64_t *l2_used,
                        uint64_t *l3_used,
                        uint32_t *queue_size);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get current time in nanoseconds
 * @return Current time in nanoseconds since epoch
 */
uint64_t raf_hd_now_ns(void);

/**
 * Align value up to alignment boundary
 * @param value Value to align
 * @param alignment Alignment boundary (must be power of 2)
 * @return Aligned value
 */
uint64_t raf_hd_align_up(uint64_t value, uint64_t alignment);

/**
 * Compute SHA-256 hash (simplified for baremetal)
 * @param data Input data
 * @param len Input length
 * @param hash Output: 32-byte hash
 */
void raf_hd_sha256(const uint8_t *data, uint32_t len, uint8_t hash[RAF_HD_HASH_SIZE]);

/**
 * Compare two event keys
 * @param k1 First key
 * @param k2 Second key
 * @return 0 if equal, non-zero if different
 */
int raf_hd_key_cmp(const raf_hd_event_key_t *k1, const raf_hd_event_key_t *k2);

/**
 * Hash event key for hash table
 * @param key Event key
 * @return Hash value
 */
uint32_t raf_hd_key_hash(const raf_hd_event_key_t *key);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_HD_CACHE_H */
