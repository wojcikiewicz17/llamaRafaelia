#pragma once

#include <cstddef>
#include <cstdint>

#if defined(LLAMA_RAF_AUDIT)
struct sa_ctx;

struct sa_opts {
    size_t chunk_size = 64 * 1024;
};

struct sa_report {
    size_t bytes_total = 0;
    uint32_t crc32c_final = 0;
    double throughput_mbps = 0.0;
    size_t io_reads = 0;
    double seconds = 0.0;
};

sa_ctx * sa_open(const char * path, sa_opts opts);
size_t sa_read(sa_ctx * ctx, void * dst, size_t n);
sa_report sa_report_ctx(const sa_ctx * ctx);
void sa_close(sa_ctx * ctx);
#else
struct sa_ctx {};
struct sa_opts {
    size_t chunk_size = 0;
};
struct sa_report {
    size_t bytes_total = 0;
    uint32_t crc32c_final = 0;
    double throughput_mbps = 0.0;
    size_t io_reads = 0;
    double seconds = 0.0;
};
inline sa_ctx * sa_open(const char *, sa_opts) { return nullptr; }
inline size_t sa_read(sa_ctx *, void *, size_t) { return 0; }
inline sa_report sa_report_ctx(const sa_ctx *) { return {}; }
inline void sa_close(sa_ctx *) {}
#endif
