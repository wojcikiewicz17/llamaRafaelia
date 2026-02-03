#include "witness_model.h"

#if defined(LLAMA_RAF_WITNESS)

#include "crc32c.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

uint64_t xorfold_bytes(const uint8_t * data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }
    uint64_t acc = 0;
    for (size_t i = 0; i < size; ++i) {
        acc ^= static_cast<uint64_t>(data[i]) << ((i % 8u) * 8u);
    }
    return acc;
}

} // namespace

witness_report witness_file(const char * path) {
    witness_report report;
    if (!path) {
        return report;
    }

    std::FILE * fp = std::fopen(path, "rb");
    if (!fp) {
        return report;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        std::fclose(fp);
        return report;
    }

    const long size = std::ftell(fp);
    if (size <= 0) {
        std::fclose(fp);
        return report;
    }

    const size_t file_size = static_cast<size_t>(size);
    const size_t sample_size = 4096;
    const size_t offsets[] = {
        0,
        file_size / 4u,
        file_size / 2u,
        (file_size * 3u) / 4u,
    };

    std::vector<uint8_t> buffer(sample_size);
    for (size_t offset : offsets) {
        if (std::fseek(fp, static_cast<long>(offset), SEEK_SET) != 0) {
            continue;
        }
        const size_t to_read = std::min(sample_size, file_size - offset);
        const size_t read_now = std::fread(buffer.data(), 1, to_read, fp);
        if (read_now == 0) {
            continue;
        }
        report.crc32c = crc32c_extend(report.crc32c, buffer.data(), read_now);
        report.xorfold ^= xorfold_bytes(buffer.data(), read_now);
        report.bytes_sampled += read_now;
    }

    std::fclose(fp);
    report.crc32c = crc32c_finalize(report.crc32c);
    report.ok = report.bytes_sampled > 0;
    return report;
}

witness_report witness_tensor(const void * data, size_t size, size_t stride) {
    witness_report report;
    if (!data || size == 0) {
        return report;
    }
    const size_t step = stride > 0 ? stride : size;
    const uint8_t * bytes = static_cast<const uint8_t *>(data);
    for (size_t offset = 0; offset < size; offset += step) {
        const size_t to_read = std::min(step, size - offset);
        report.crc32c = crc32c_extend(report.crc32c, bytes + offset, to_read);
        report.xorfold ^= xorfold_bytes(bytes + offset, to_read);
        report.bytes_sampled += to_read;
    }
    report.crc32c = crc32c_finalize(report.crc32c);
    report.ok = report.bytes_sampled > 0;
    return report;
}

std::string witness_signature(const witness_report & report) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << report.crc32c
        << ":"
        << std::setw(16) << report.xorfold;
    return oss.str();
}

#endif
