# HD Cache MVP - Hierarchical Disk Cache Module

## Overview

The HD Cache MVP module provides a high-performance, hierarchical disk cache system with L1/L2/L3 memory tiers, TTL-based expiration, retry logic, and harmonic scheduling for event-driven processing. It is designed as a baremetal C module with zero external dependencies beyond the standard library.

## Architecture

### Core Components

1. **Block Store**: Append-only storage with page alignment
2. **L1/L2/L3 Cache**: Three-tier memory cache with automatic demotion
3. **Harmonic Scheduler**: Weighted event processing based on layer frequencies
4. **Metadata Store**: Fast hash-table based event metadata lookup
5. **Event Queue System**: Per-layer FIFO queues with overflow protection

### Block Store

The block store provides persistent, append-only storage with the following features:

- **Page Alignment**: Blocks are aligned to 4KB boundaries for optimal disk I/O
- **Checksumming**: FNV-based hash for integrity verification
- **Block Format**:
  ```
  [MAGIC 4B][VER 1B][FLAGS 1B][RESV 2B][PAYLEN 4B][HASH 32B]
  [PAYLOAD ...][PADDING to 4KB boundary]
  ```
- **Maximum Block Size**: 256KB payload per block
- **Index File**: JSON Lines format for metadata persistence

### Cache Hierarchy

Three-tier LRU cache with byte budgets:

| Tier | Budget | Purpose | Eviction Policy |
|------|--------|---------|-----------------|
| L1   | 2 MB   | Hot (most recently used) | FIFO when over budget, demote to L2 |
| L2   | 16 MB  | Warm (recently accessed) | FIFO when over budget, demote to L3 |
| L3   | 128 MB | Cold (infrequently used) | FIFO when over budget, evict completely |

**Cache Operations:**
- `put_hot`: Insert entry into L1 (hot tier)
- `get`: Lookup in L1 → L2 → L3 (no automatic promotion to prevent thrashing)
- `demote_cycle`: Move overflow entries down tiers

### Harmonic Scheduler

Deterministic, frequency-based scheduler for processing events across layers:

- **Frequency Weights**: Higher frequency layers get more processing slots
  - freq >= 100,000: weight 64 (e.g., IRQ-like high-priority)
  - freq >= 1,000: weight 16
  - freq >= 500: weight 8
  - freq >= 100: weight 4
  - freq < 100: weight 2
  
- **Scheduling Algorithm**: Weighted round-robin with deterministic rotation
- **Cycle Progression**: Tick counter for reproducible scheduling

### Event Lifecycle

1. **Ingest**: Event is stored to disk, indexed, cached in L1, and enqueued
2. **Schedule**: Harmonic scheduler selects next layer based on weights
3. **Process**: Event is dequeued, payload fetched (cache or disk), processed
4. **Result Handling**:
   - Success: Mark as DONE
   - Failure: Retry (up to 3 attempts) or DROP
   - Expired: Mark as EXPIRED if past TTL
   
### TTL and Retry Logic

- **Default TTL**: 30 seconds (configurable per event)
- **Max Retries**: 3 attempts before dropping
- **Status States**:
  - NEW: Just ingested
  - HOT/COLD: In cache tiers
  - EXPIRED: Past TTL
  - RETRYING: Failed, retrying
  - DROPPED: Exceeded retries or queue overflow
  - DONE: Successfully processed

### Queue Management

- **Per-Layer Queues**: FIFO queues for each layer
- **Overflow Protection**: Drops oldest event from largest queue when total exceeds 10,000
- **Requeue Logic**: Failed events are requeued at the end for retry

## API Reference

### Initialization

```c
/* Create HD cache engine */
raf_hd_cache_t* raf_hd_cache_create(const char *store_path, const char *index_path);

/* Add processing layer */
int raf_hd_cache_add_layer(raf_hd_cache_t *cache, const char *layer, uint32_t frequency);

/* Destroy engine */
void raf_hd_cache_destroy_engine(raf_hd_cache_t *cache);
```

### Event Ingestion

```c
/* Ingest event into cache */
int raf_hd_cache_ingest(raf_hd_cache_t *cache,
                        const char *layer,
                        const uint8_t *payload,
                        uint32_t payload_len,
                        uint64_t ttl_ns,
                        raf_hd_event_key_t *key);
```

### Event Processing

```c
/* Processing callback signature */
typedef int (*raf_hd_process_fn)(const raf_hd_event_key_t *key, 
                                  const uint8_t *payload, 
                                  uint32_t payload_len,
                                  void *user_data);

/* Process events for one tick */
uint32_t raf_hd_cache_tick(raf_hd_cache_t *cache,
                           uint32_t max_events,
                           raf_hd_process_fn process_fn,
                           void *user_data);
```

### Statistics

```c
/* Get cache statistics */
void raf_hd_cache_stats(raf_hd_cache_t *cache,
                        uint64_t *l1_used,
                        uint64_t *l2_used,
                        uint64_t *l3_used,
                        uint32_t *queue_size);
```

## Usage Example

```c
#include "raf_hd_cache.h"

/* Processing callback */
static int process_event(const raf_hd_event_key_t *key, 
                        const uint8_t *payload, 
                        uint32_t payload_len,
                        void *user_data) {
    /* Process the event */
    printf("Processing %s/%s: %u bytes\n", key->layer, key->eid, payload_len);
    return 0; /* Success */
}

int main(void) {
    /* Create cache */
    raf_hd_cache_t *cache = raf_hd_cache_create("/tmp/store.dat", "/tmp/index.jsonl");
    
    /* Add layers with harmonic frequencies */
    raf_hd_cache_add_layer(cache, "past", 144);
    raf_hd_cache_add_layer(cache, "present", 288);
    raf_hd_cache_add_layer(cache, "future", 777);
    raf_hd_cache_add_layer(cache, "irq_hot", 144000);
    
    /* Ingest events */
    for (int i = 0; i < 100; i++) {
        char payload[256];
        snprintf(payload, sizeof(payload), "Event %d data", i);
        
        raf_hd_event_key_t key;
        raf_hd_cache_ingest(cache, "present", 
                           (uint8_t*)payload, strlen(payload),
                           0, &key);
    }
    
    /* Process events in ticks */
    for (int tick = 0; tick < 10; tick++) {
        uint32_t processed = raf_hd_cache_tick(cache, 20, process_event, NULL);
        printf("Tick %d: processed %u events\n", tick, processed);
    }
    
    /* Cleanup */
    raf_hd_cache_destroy_engine(cache);
    return 0;
}
```

## Building

The HD Cache module is part of the rafaelia-baremetal library:

```bash
# Build with rafaelia-baremetal enabled
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build

# Run example
./build/bin/rafaelia-hd-cache-example
```

## Performance Characteristics

- **Block Write**: O(1) append with fsync
- **Block Read**: O(1) seek + read with integrity check
- **Cache Lookup**: O(n) linear search within tier (n = entries in tier)
- **Metadata Lookup**: O(1) hash table lookup
- **Event Processing**: O(w) where w = sum of layer weights
- **Memory Usage**: L1 + L2 + L3 budgets + metadata overhead

## Design Principles

1. **Baremetal**: No external dependencies beyond C standard library
2. **Deterministic**: Reproducible behavior with fixed seed/configuration
3. **Hardware-Aware**: Page-aligned I/O, cache-friendly data structures
4. **Zero-Copy**: Minimizes data copies where possible
5. **Fail-Safe**: Retry logic and overflow protection
6. **Observable**: Statistics and status tracking

## Integration with llama.cpp

The HD Cache can be used in llama.cpp for:

- **KV Cache Persistence**: Store key-value cache blocks to disk
- **Request Queueing**: Queue inference requests with priority
- **Result Caching**: Cache inference results with TTL
- **Batch Processing**: Process batches with harmonic scheduling

Example integration:

```c
/* In llama inference loop */
raf_hd_cache_t *request_cache = raf_hd_cache_create("requests.dat", "requests.idx");
raf_hd_cache_add_layer(request_cache, "high_priority", 1000);
raf_hd_cache_add_layer(request_cache, "normal", 100);
raf_hd_cache_add_layer(request_cache, "low_priority", 10);

/* Ingest request */
raf_hd_event_key_t key;
raf_hd_cache_ingest(request_cache, "high_priority", 
                   request_data, request_len, 
                   30 * 1000000000ULL, &key);

/* Process in main loop */
raf_hd_cache_tick(request_cache, 10, process_inference_request, llama_ctx);
```

## Future Enhancements

- [ ] Lock-free concurrent access
- [ ] mmap-backed L3 cache
- [ ] Compression integration (ZIPRAF)
- [ ] Bloom filters for negative lookups
- [ ] Adaptive TTL based on access patterns
- [ ] Per-layer statistics and metrics
- [ ] Integration with BITSTACK for sparse operations

## License

MIT License - See LICENSE file

Copyright (c) 2026 Rafael Melo Reis
