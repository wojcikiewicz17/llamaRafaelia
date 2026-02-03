#pragma once

#include <cstddef>
#include <cstdint>

namespace bitstack {

enum class witness_mode {
    xor_fold,
    crc32c,
};

enum class fallback_mode {
    none,
    zero,
    quarantine,
};

struct q4_block_view {
    const uint8_t * weights = nullptr;
    size_t weights_bytes = 0;
    const uint8_t * meta = nullptr;
    size_t meta_bytes = 0;
    uint32_t witness = 0;
};

struct witness_config {
    witness_mode mode = witness_mode::xor_fold;
    bool warmup = false;
    fallback_mode fallback = fallback_mode::none;
};

struct witness_report {
    bool ok = true;
    uint32_t computed = 0;
};

uint32_t compute_witness(const uint8_t * data, size_t size, witness_mode mode);
uint32_t compute_q4_witness(const q4_block_view & view, witness_mode mode);

witness_report verify_q4_block(const q4_block_view & view, const witness_config & config);

void warmup_pages(const void * data, size_t size);
void apply_fallback(uint8_t * data, size_t size, fallback_mode mode);

} // namespace bitstack
