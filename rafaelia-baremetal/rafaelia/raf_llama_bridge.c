#include "rafaelia.h"

#include "raf_zipraf.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define raf_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define raf_mkdir(path) mkdir(path, 0755)
#endif

static char g_cache_dir[512] = {0};
static char g_policy_path[512] = {0};
static char g_bitstack_log_path[512] = {0};

static void rafstore_set_path(char *dst, size_t dst_size, const char *value) {
    if (!dst || dst_size == 0) {
        return;
    }
    if (!value) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, value, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

int rafstore_init(const char *cache_dir, const char *policy) {
    rafstore_set_path(g_cache_dir, sizeof(g_cache_dir), cache_dir);
    rafstore_set_path(g_policy_path, sizeof(g_policy_path), policy);

    if (g_cache_dir[0] != '\0') {
        if (raf_mkdir(g_cache_dir) != 0) {
            if (errno != EEXIST) {
                return -1;
            }
        }
    }

    return 0;
}

int rafstore_prefetch(const char *model_path, size_t window_mb, const char *strategy) {
    (void) strategy;

    if (!model_path || model_path[0] == '\0') {
        return -1;
    }

    size_t window_bytes = window_mb > 0 ? window_mb * 1024u * 1024u : 64u * 1024u * 1024u;

    FILE *fp = fopen(model_path, "rb");
    if (!fp) {
        return -1;
    }

    const size_t chunk_size = 1024u * 1024u;
    uint8_t *buffer = (uint8_t *) malloc(chunk_size);
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    size_t total_read = 0;
    while (total_read < window_bytes) {
        size_t to_read = chunk_size;
        if (window_bytes - total_read < chunk_size) {
            to_read = window_bytes - total_read;
        }
        size_t read_now = fread(buffer, 1, to_read, fp);
        total_read += read_now;
        if (read_now < to_read) {
            break;
        }
    }

    free(buffer);
    fclose(fp);
    return 0;
}

void bitstack_set_log_path(const char *path) {
    rafstore_set_path(g_bitstack_log_path, sizeof(g_bitstack_log_path), path);
}

int bitstack_append(const char *event_json) {
    if (!event_json || event_json[0] == '\0') {
        return -1;
    }

    if (g_bitstack_log_path[0] == '\0') {
        return 0;
    }

    FILE *fp = fopen(g_bitstack_log_path, "a");
    if (!fp) {
        return -1;
    }

    fputs(event_json, fp);
    fputc('\n', fp);
    fclose(fp);
    return 0;
}

uint32_t bitraf_crc32c(const uint8_t *data, size_t size) {
    return raf_zipraf_crc32(data, size);
}

uint64_t bitraf_xorfold(const uint8_t *data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }
    uint64_t acc = 0;
    for (size_t i = 0; i < size; ++i) {
        acc ^= ((uint64_t) data[i] << ((i % 8u) * 8u));
    }
    return acc;
}
