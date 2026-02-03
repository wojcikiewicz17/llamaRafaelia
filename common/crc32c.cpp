#include "crc32c.h"

namespace {

constexpr uint32_t kCrc32cPolynomial = 0x82f63b78u;

uint32_t g_table[256];
bool g_table_init = false;

void init_table() {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; ++j) {
            if (crc & 1u) {
                crc = (crc >> 1u) ^ kCrc32cPolynomial;
            } else {
                crc >>= 1u;
            }
        }
        g_table[i] = crc;
    }
    g_table_init = true;
}

} // namespace

uint32_t crc32c_extend(uint32_t crc, const uint8_t * data, size_t size) {
    if (!g_table_init) {
        init_table();
    }
    crc = ~crc;
    for (size_t i = 0; i < size; ++i) {
        crc = g_table[(crc ^ data[i]) & 0xffu] ^ (crc >> 8u);
    }
    return ~crc;
}

uint32_t crc32c_finalize(uint32_t crc) {
    return crc;
}

uint32_t crc32c_compute(const uint8_t * data, size_t size) {
    return crc32c_finalize(crc32c_extend(0, data, size));
}
