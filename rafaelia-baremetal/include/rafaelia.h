#ifndef RAFAELIA_LLAMA_BRIDGE_H
#define RAFAELIA_LLAMA_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "licensing/raf_licensing.h"

#ifdef __cplusplus
extern "C" {
#endif

int rafstore_init(const char *cache_dir, const char *policy);
int rafstore_prefetch(const char *model_path, size_t window_mb, const char *strategy);

void bitstack_set_log_path(const char *path);
int bitstack_append(const char *event_json);

uint32_t bitraf_crc32c(const uint8_t *data, size_t size);
uint64_t bitraf_xorfold(const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_LLAMA_BRIDGE_H */
