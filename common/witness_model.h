#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#if defined(LLAMA_RAF_WITNESS)
struct witness_report {
    uint32_t crc32c = 0;
    uint64_t xorfold = 0;
    size_t bytes_sampled = 0;
    bool ok = false;
};

witness_report witness_file(const char * path);
witness_report witness_tensor(const void * data, size_t size, size_t stride);
std::string witness_signature(const witness_report & report);
#else
struct witness_report {
    uint32_t crc32c = 0;
    uint64_t xorfold = 0;
    size_t bytes_sampled = 0;
    bool ok = false;
};
inline witness_report witness_file(const char *) { return {}; }
inline witness_report witness_tensor(const void *, size_t, size_t) { return {}; }
inline std::string witness_signature(const witness_report &) { return ""; }
#endif
