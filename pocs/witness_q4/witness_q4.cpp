#include "witness_q4.h"

#include <cstring>

namespace bitstack {
namespace {

uint32_t xor_fold_update(uint32_t acc, const uint8_t * data, size_t size) {
    if (!data || size == 0) {
        return acc;
    }
    for (size_t i = 0; i < size; ++i) {
        const uint32_t shift = static_cast<uint32_t>((i % 4) * 8);
        acc ^= static_cast<uint32_t>(data[i]) << shift;
    }
    return acc;
}

uint32_t crc32c_update(uint32_t crc, const uint8_t * data, size_t size) {
    if (!data || size == 0) {
        return crc;
    }
    crc = ~crc;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int k = 0; k < 8; ++k) {
            const uint32_t mask = static_cast<uint32_t>(-(static_cast<int>(crc & 1)));
            crc = (crc >> 1) ^ (0x82F63B78u & mask);
        }
    }
    return ~crc;
}

} // namespace

uint32_t compute_witness(const uint8_t * data, size_t size, witness_mode mode) {
    if (mode == witness_mode::crc32c) {
        return crc32c_update(0u, data, size);
    }
    return xor_fold_update(0u, data, size);
}

uint32_t compute_q4_witness(const q4_block_view & view, witness_mode mode) {
    if (mode == witness_mode::crc32c) {
        uint32_t crc = crc32c_update(0u, view.weights, view.weights_bytes);
        return crc32c_update(crc, view.meta, view.meta_bytes);
    }
    uint32_t acc = xor_fold_update(0u, view.weights, view.weights_bytes);
    return xor_fold_update(acc, view.meta, view.meta_bytes);
}

witness_report verify_q4_block(const q4_block_view & view, const witness_config & config) {
    if (config.warmup) {
        warmup_pages(view.weights, view.weights_bytes);
        warmup_pages(view.meta, view.meta_bytes);
    }
    witness_report report;
    report.computed = compute_q4_witness(view, config.mode);
    report.ok = (report.computed == view.witness);
    return report;
}

witness_report verify_q4_block(q4_block_mut_view & view, const witness_config & config) {
    const q4_block_view ro_view{
        view.weights,
        view.weights_bytes,
        view.meta,
        view.meta_bytes,
        view.witness,
    };
    witness_report report = verify_q4_block(ro_view, config);
    if (!report.ok) {
        apply_fallback(view.weights, view.weights_bytes, config.fallback);
        apply_fallback(view.meta, view.meta_bytes, config.fallback);
    }
    return report;
}

void warmup_pages(const void * data, size_t size) {
    if (!data || size == 0) {
        return;
    }
    const size_t page = 4096;
    const uint8_t * bytes = static_cast<const uint8_t *>(data);
    volatile uint8_t sink = 0;
    for (size_t offset = 0; offset < size; offset += page) {
        sink ^= bytes[offset];
    }
    sink ^= bytes[size - 1];
    (void) sink;
}

void apply_fallback(uint8_t * data, size_t size, fallback_mode mode) {
    if (!data || size == 0 || mode == fallback_mode::none) {
        return;
    }
    if (mode == fallback_mode::zero) {
        std::memset(data, 0, size);
        return;
    }
    if (mode == fallback_mode::quarantine) {
        std::memset(data, 0xFF, size);
    }
}

} // namespace bitstack
