#include "stream_audit.h"

#if defined(LLAMA_RAF_AUDIT)

#include "crc32c.h"

#include <chrono>
#include <cstdio>

struct sa_ctx {
    std::FILE * fp = nullptr;
    size_t bytes_total = 0;
    size_t io_reads = 0;
    uint32_t crc32c = 0;
    size_t chunk_size = 0;
    std::chrono::steady_clock::time_point start;
};

sa_ctx * sa_open(const char * path, sa_opts opts) {
    if (!path) {
        return nullptr;
    }
    std::FILE * fp = std::fopen(path, "rb");
    if (!fp) {
        return nullptr;
    }
    sa_ctx * ctx = new sa_ctx();
    ctx->fp = fp;
    ctx->chunk_size = opts.chunk_size > 0 ? opts.chunk_size : 64 * 1024;
    ctx->start = std::chrono::steady_clock::now();
    return ctx;
}

size_t sa_read(sa_ctx * ctx, void * dst, size_t n) {
    if (!ctx || !ctx->fp || !dst || n == 0) {
        return 0;
    }
    size_t read_now = std::fread(dst, 1, n, ctx->fp);
    if (read_now > 0) {
        ctx->crc32c = crc32c_extend(ctx->crc32c, static_cast<const uint8_t *>(dst), read_now);
        ctx->bytes_total += read_now;
        ctx->io_reads += 1;
    }
    return read_now;
}

sa_report sa_report_ctx(const sa_ctx * ctx) {
    sa_report report;
    if (!ctx) {
        return report;
    }
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end - ctx->start;
    report.bytes_total = ctx->bytes_total;
    report.crc32c_final = crc32c_finalize(ctx->crc32c);
    report.io_reads = ctx->io_reads;
    report.seconds = elapsed.count();
    if (report.seconds > 0.0) {
        report.throughput_mbps = (static_cast<double>(report.bytes_total) / (1024.0 * 1024.0)) / report.seconds;
    }
    return report;
}

void sa_close(sa_ctx * ctx) {
    if (!ctx) {
        return;
    }
    if (ctx->fp) {
        std::fclose(ctx->fp);
        ctx->fp = nullptr;
    }
    delete ctx;
}

#endif
