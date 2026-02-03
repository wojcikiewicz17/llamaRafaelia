#include "bitstack_log.h"

#if defined(LLAMA_RAF_BITSTACK)

#include "crc32c.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

namespace {

std::mutex g_mutex;
std::string g_path;

std::string json_escape(const char * value) {
    if (!value) {
        return "";
    }
    std::string out;
    out.reserve(std::strlen(value) + 8);
    for (const char * p = value; *p; ++p) {
        switch (*p) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += *p; break;
        }
    }
    return out;
}

} // namespace

void bs_init(const char * path) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_path = path ? path : "";
}

void bs_event(const char * tag, int code, const char * msg) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_path.empty() || !tag) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::string payload = std::string("{\"ts\":") + std::to_string(epoch_ms)
        + ",\"tag\":\"" + json_escape(tag) + "\""
        + ",\"code\":" + std::to_string(code)
        + ",\"msg\":\"" + json_escape(msg ? msg : "") + "\"}";

    const uint32_t crc32c = crc32c_compute(reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
    std::string record = payload.substr(0, payload.size() - 1) + ",\"crc32c\":" + std::to_string(crc32c) + "}";

    std::FILE * fp = std::fopen(g_path.c_str(), "a");
    if (!fp) {
        return;
    }
    std::fwrite(record.data(), 1, record.size(), fp);
    std::fputc('\n', fp);
    std::fclose(fp);
}

void bs_flush() {
    std::lock_guard<std::mutex> lock(g_mutex);
}

#endif
