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

struct metadata {
    std::unordered_map<std::string, std::string> fields;
};

struct result {
    int risk_level = 0; // 0..3
    action action_taken = action::allow;
    std::vector<std::string> reasons;
};

result evaluate(const std::string & prompt, const metadata & meta);

const std::string & witness_safe();
const std::string & witness_block();

std::string action_to_string(action value);
std::string render_message(const result & res);

} // namespace smart_guard
