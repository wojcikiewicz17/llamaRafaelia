#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace smart_guard {

enum class action {
    allow,
    warn,
    block,
};

enum class mode {
    warn,
    block,
};

struct metadata {
    std::unordered_map<std::string, std::string> fields;
};

struct result {
    int risk_level = 0; // 0..3
    action action_taken = action::allow;
    std::vector<std::string> reasons;
    std::string reason_code;
    std::vector<std::string> categories;
    std::string normalized_prompt_digest;
};

result evaluate(const std::string & prompt, const metadata & meta);
result apply_mode(const result & res, mode guard_mode);

const std::string & witness_safe();
const std::string & witness_block();

std::string action_to_string(action value);
std::string mode_to_string(mode value);
mode mode_from_string(const std::string & value);
std::string render_message(const result & res);
std::string render_metadata_json(const result & res);

} // namespace smart_guard
