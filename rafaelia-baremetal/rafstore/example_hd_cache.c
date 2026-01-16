/**
 * Rafaelia Baremetal - HD Cache Example
 * 
 * Demonstrates usage of the hierarchical disk cache system with
 * L1/L2/L3 tiers, TTL, retry logic, and harmonic scheduling.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_hd_cache.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* ANSI color codes for terminal output */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_BOLD    "\x1b[1m"

/* Event processing callback */
static int process_event(const raf_hd_event_key_t *key, 
                        const uint8_t *payload, 
                        uint32_t payload_len,
                        void *user_data) {
    (void)user_data; /* Unused */
    
    /* Simulate "matrix op" - deterministic transformation */
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < payload_len; i++) {
        checksum = ((checksum << 5) + checksum) + payload[i];
    }
    
    printf("  " COLOR_GREEN "✓" COLOR_RESET " Processed event [%s/%s] len=%u checksum=%08x\n",
           key->layer, key->eid, payload_len, checksum);
    
    /* Simulate occasional failures for retry testing */
    if (checksum % 100 == 13) {
        return -1; /* Failure - will retry */
    }
    
    return 0; /* Success */
}

static void print_separator(void) {
    printf(COLOR_CYAN "════════════════════════════════════════════════════════════════\n" COLOR_RESET);
}

static void print_header(const char *title) {
    print_separator();
    printf(COLOR_BOLD COLOR_YELLOW "%s\n" COLOR_RESET, title);
    print_separator();
}

static void print_stats(raf_hd_cache_t *cache) {
    uint64_t l1_used, l2_used, l3_used;
    uint32_t queue_size;
    
    raf_hd_cache_stats(cache, &l1_used, &l2_used, &l3_used, &queue_size);
    
    printf(COLOR_CYAN "Cache Statistics:\n" COLOR_RESET);
    printf("  L1 (hot):  %6llu KB / %4llu KB\n", 
           (unsigned long long)(l1_used / 1024), 
           (unsigned long long)(RAF_HD_L1_BUDGET / 1024));
    printf("  L2 (warm): %6llu KB / %4llu KB\n", 
           (unsigned long long)(l2_used / 1024), 
           (unsigned long long)(RAF_HD_L2_BUDGET / 1024));
    printf("  L3 (cold): %6llu KB / %4llu KB\n", 
           (unsigned long long)(l3_used / 1024), 
           (unsigned long long)(RAF_HD_L3_BUDGET / 1024));
    printf("  Queue:     %u events\n", queue_size);
    printf("  Total ingested: %llu\n", (unsigned long long)cache->total_events);
    printf("  Dropped: %llu\n", (unsigned long long)cache->dropped_events);
}

int main(void) {
    srand((unsigned int)time(NULL));
    
    print_header("Rafaelia HD Cache MVP - Example");
    
    /* Create cache engine */
    printf(COLOR_YELLOW "\n► Creating HD cache engine...\n" COLOR_RESET);
    
    const char *store_path = "/tmp/rafaelia_store.dat";
    const char *index_path = "/tmp/rafaelia_index.jsonl";
    
    /* Remove old files */
    remove(store_path);
    remove(index_path);
    
    raf_hd_cache_t *cache = raf_hd_cache_create(store_path, index_path);
    if (!cache) {
        fprintf(stderr, COLOR_RED "Error: Failed to create cache\n" COLOR_RESET);
        return 1;
    }
    
    printf(COLOR_GREEN "  ✓ Cache created successfully\n" COLOR_RESET);
    printf("  Store: %s\n", store_path);
    printf("  Index: %s\n", index_path);
    
    /* Add layers with harmonic frequencies */
    print_header("1. Adding Layers with Harmonic Frequencies");
    
    const struct {
        const char *name;
        uint32_t freq;
    } layers[] = {
        {"past",        144},
        {"present",     288},
        {"future",      777},
        {"irq_hot",     144000},
        {"noise_band",  12},
        {"f963",        963},
        {"f999",        999},
    };
    
    for (size_t i = 0; i < sizeof(layers)/sizeof(layers[0]); i++) {
        if (raf_hd_cache_add_layer(cache, layers[i].name, layers[i].freq) != 0) {
            fprintf(stderr, COLOR_RED "Error: Failed to add layer %s\n" COLOR_RESET, 
                    layers[i].name);
            raf_hd_cache_destroy_engine(cache);
            return 1;
        }
        printf("  " COLOR_GREEN "✓" COLOR_RESET " Added layer: %-12s (freq=%u)\n", 
               layers[i].name, layers[i].freq);
    }
    
    /* Ingest sample events */
    print_header("2. Ingesting Events");
    
    const uint32_t NUM_EVENTS = 500;
    printf("  Ingesting %u events across all layers...\n", NUM_EVENTS);
    
    for (uint32_t i = 0; i < NUM_EVENTS; i++) {
        /* Select random layer */
        size_t layer_idx = (size_t)rand() % (sizeof(layers)/sizeof(layers[0]));
        const char *layer = layers[layer_idx].name;
        
        /* Create payload */
        char payload_buf[2048];
        uint32_t payload_len = snprintf(payload_buf, sizeof(payload_buf),
                                       "%s|i=%u|t=%llu|%s",
                                       layer, i, 
                                       (unsigned long long)raf_hd_now_ns(),
                                       "ABCDEFGHIJ");
        
        /* Make payload size vary */
        payload_len = payload_len + (i % 512);
        if (payload_len > sizeof(payload_buf)) payload_len = sizeof(payload_buf);
        
        /* Ingest event */
        raf_hd_event_key_t key;
        if (raf_hd_cache_ingest(cache, layer, (const uint8_t*)payload_buf, 
                                payload_len, 0, &key) != 0) {
            fprintf(stderr, COLOR_RED "Error: Failed to ingest event %u\n" COLOR_RESET, i);
            continue;
        }
        
        if (i % 100 == 0 || i < 5 || i >= NUM_EVENTS - 5) {
            printf("  " COLOR_GREEN "✓" COLOR_RESET " Ingested: [%s/%s] %u bytes\n",
                   key.layer, key.eid, payload_len);
        } else if (i == 5) {
            printf("  ...\n");
        }
    }
    
    printf(COLOR_GREEN "\n  ✓ Ingestion complete\n" COLOR_RESET);
    print_stats(cache);
    
    /* Process events with harmonic scheduling */
    print_header("3. Processing Events with Harmonic Scheduler");
    
    const uint32_t NUM_TICKS = 50;
    const uint32_t EVENTS_PER_TICK = 20;
    
    printf("  Running %u ticks, processing up to %u events per tick...\n\n",
           NUM_TICKS, EVENTS_PER_TICK);
    
    for (uint32_t tick = 0; tick < NUM_TICKS; tick++) {
        uint32_t processed = raf_hd_cache_tick(cache, EVENTS_PER_TICK, 
                                              process_event, NULL);
        
        if (tick < 5 || tick >= NUM_TICKS - 5) {
            printf(COLOR_CYAN "Tick %2u:" COLOR_RESET " processed %u events\n", 
                   tick, processed);
        } else if (tick == 5) {
            printf("  ...\n");
        }
        
        /* Show stats every 10 ticks */
        if (tick % 10 == 9) {
            printf("\n");
            print_stats(cache);
            printf("\n");
        }
    }
    
    printf(COLOR_GREEN "\n  ✓ Processing complete\n" COLOR_RESET);
    
    /* Final statistics */
    print_header("4. Final Statistics");
    print_stats(cache);
    
    /* Test cache retrieval */
    print_header("5. Testing Cache Retrieval");
    
    printf("  Testing cache hits/misses...\n");
    
    /* Try to access some events (should be in cache or on disk) */
    raf_hd_event_key_t test_key;
    strncpy(test_key.layer, "present", sizeof(test_key.layer));
    strncpy(test_key.eid, "test123", sizeof(test_key.eid));
    
    uint8_t *data = NULL;
    uint32_t data_len = 0;
    
    int result = raf_hd_cache_get(&cache->cache, &test_key, &data, &data_len);
    if (result == 0) {
        printf("  " COLOR_GREEN "✓" COLOR_RESET " Cache hit for test key (found in cache)\n");
    } else {
        printf("  " COLOR_YELLOW "○" COLOR_RESET " Cache miss for test key (expected, not ingested)\n");
    }
    
    /* Test block store integrity */
    print_header("6. Testing Block Store Integrity");
    
    printf("  Testing append and read operations...\n");
    
    const char *test_payload = "Test payload for integrity check";
    uint32_t test_len = (uint32_t)strlen(test_payload);
    uint64_t offset;
    uint32_t disk_len;
    uint8_t hash[RAF_HD_HASH_SIZE];
    
    if (raf_hd_store_append(&cache->store, (const uint8_t*)test_payload, 
                            test_len, &offset, &disk_len, hash) == 0) {
        printf("  " COLOR_GREEN "✓" COLOR_RESET " Appended test block (offset=%llu, len=%u)\n",
               (unsigned long long)offset, disk_len);
        
        /* Read it back */
        uint8_t *read_payload = NULL;
        uint32_t read_len = 0;
        
        if (raf_hd_store_read(&cache->store, offset, &read_payload, &read_len) == 0) {
            if (read_len == test_len && memcmp(read_payload, test_payload, test_len) == 0) {
                printf("  " COLOR_GREEN "✓" COLOR_RESET " Read back successfully - integrity verified\n");
            } else {
                printf("  " COLOR_RED "✗" COLOR_RESET " Data mismatch after read\n");
            }
            free(read_payload);
        } else {
            printf("  " COLOR_RED "✗" COLOR_RESET " Failed to read block\n");
        }
    } else {
        printf("  " COLOR_RED "✗" COLOR_RESET " Failed to append block\n");
    }
    
    /* Cleanup */
    print_header("7. Cleanup");
    
    printf("  Destroying cache engine...\n");
    raf_hd_cache_destroy_engine(cache);
    printf(COLOR_GREEN "  ✓ Cache destroyed\n" COLOR_RESET);
    
    printf("\n  Generated files:\n");
    printf("    • %s (block store)\n", store_path);
    printf("    • %s (index)\n", index_path);
    
    print_separator();
    printf(COLOR_BOLD COLOR_GREEN "\n✓ HD Cache MVP Example Completed Successfully\n" COLOR_RESET);
    print_separator();
    
    printf("\nKey Features Demonstrated:\n");
    printf("  ✓ Append-only block store with page alignment\n");
    printf("  ✓ L1/L2/L3 tiered cache with automatic demotion\n");
    printf("  ✓ Harmonic scheduler with weighted layer processing\n");
    printf("  ✓ Event ingestion with TTL tracking\n");
    printf("  ✓ Retry logic for failed operations\n");
    printf("  ✓ Queue overflow protection with dropping\n");
    printf("  ✓ Hash-based integrity verification\n");
    printf("  ✓ Metadata persistence to index file\n");
    
    return 0;
}
