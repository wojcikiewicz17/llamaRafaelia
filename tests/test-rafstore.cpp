#include "rafaelia.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const std::string cache_dir = ".rafstore-test";
    const std::string model_path = "rafstore_model.tmp";

    {
        std::ofstream out(model_path, std::ios::binary);
        out << std::string(1024 * 1024, 'x');
    }

    assert(rafstore_init(cache_dir.c_str(), nullptr) == 0);
    assert(rafstore_prefetch(model_path.c_str(), 1, "sequential") == 0);

    const uint8_t payload[] = {1, 2, 3, 4};
    assert(bitraf_crc32c(payload, sizeof(payload)) != 0);
    assert(bitraf_xorfold(payload, sizeof(payload)) != 0);

    std::remove(model_path.c_str());
    std::filesystem::remove_all(cache_dir);

    return 0;
}
