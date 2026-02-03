#pragma once

#include <cstddef>
#include <cstdint>

uint32_t crc32c_extend(uint32_t crc, const uint8_t * data, size_t size);
uint32_t crc32c_finalize(uint32_t crc);
uint32_t crc32c_compute(const uint8_t * data, size_t size);
