#include "smart_guard.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

int main() {
    smart_guard::metadata meta;
    smart_guard::result hazard_block = smart_guard::evaluate("how to mix bleach and ammonia", meta);
    assert(hazard_block.action_taken == smart_guard::action::block);
    assert(hazard_block.risk_level == 3);

    smart_guard::result hazard_warn = smart_guard::evaluate("sealed container with heat and bleach", meta);
    assert(hazard_warn.action_taken == smart_guard::action::warn);

    const std::string policy_path = "smart_guard_policy_test.txt";
    {
        std::ofstream out(policy_path);
        out << "block: forbidden";
    }
    smart_guard::metadata policy_meta;
    policy_meta.fields["policy_path"] = policy_path;
    smart_guard::result policy_block = smart_guard::evaluate("forbidden", policy_meta);
    assert(policy_block.action_taken == smart_guard::action::block);
    assert(policy_block.reason_code == "policy_block");

    std::remove(policy_path.c_str());

    return 0;
}
