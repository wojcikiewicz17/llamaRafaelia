#include "smart_guard.h"
#include "synonym_normalizer.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace smart_guard {
namespace {

bool contains_any(const std::string & text, const std::vector<std::string> & phrases) {
    for (const auto & phrase : phrases) {
        if (text.find(phrase) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string trim_copy(const std::string & input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }
    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }
    return input.substr(start, end - start);
}

struct policy_terms {
    std::vector<std::string> warn_terms;
    std::vector<std::string> block_terms;
};

policy_terms load_policy_terms(const std::string & path) {
    policy_terms terms;
    if (path.empty()) {
        return terms;
    }

    std::ifstream infile(path);
    if (!infile.is_open()) {
        return terms;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::string trimmed = trim_copy(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        const std::string warn_prefix = "warn:";
        const std::string block_prefix = "block:";
        if (trimmed.rfind(warn_prefix, 0) == 0) {
            std::string term = trim_copy(trimmed.substr(warn_prefix.size()));
            if (!term.empty()) {
                terms.warn_terms.push_back(normalize_text(term));
            }
            continue;
        }
        if (trimmed.rfind(block_prefix, 0) == 0) {
            std::string term = trim_copy(trimmed.substr(block_prefix.size()));
            if (!term.empty()) {
                terms.block_terms.push_back(normalize_text(term));
            }
            continue;
        }

        terms.warn_terms.push_back(normalize_text(trimmed));
    }

    return terms;
}

std::vector<std::string> collect_reasons(bool vulnerable, bool ambiguous, bool hazard_request, bool hazard_context, bool allergy_trigger) {
    std::vector<std::string> reasons;
    if (vulnerable) {
        reasons.emplace_back("vulnerable_context");
    }
    if (ambiguous) {
        reasons.emplace_back("ambiguous_request");
    }
    if (hazard_request) {
        reasons.emplace_back("explicit_hazard_request");
    }
    if (hazard_context) {
        reasons.emplace_back("hazard_context");
    }
    if (allergy_trigger) {
        reasons.emplace_back("allergy_sensitive_term");
    }
    return reasons;
}

std::vector<std::string> collect_categories(const std::vector<std::string> & reasons) {
    std::vector<std::string> categories;
    auto add_category = [&categories](const std::string & value) {
        if (std::find(categories.begin(), categories.end(), value) == categories.end()) {
            categories.push_back(value);
        }
    };

    for (const auto & reason : reasons) {
        if (reason == "vulnerable_context") {
            add_category("vulnerable");
        } else if (reason == "ambiguous_request") {
            add_category("ambiguous");
        } else if (reason == "explicit_hazard_request" || reason == "hazard_context") {
            add_category("hazard");
        } else if (reason == "allergy_sensitive_term") {
            add_category("allergy");
        } else if (reason == "policy_warn" || reason == "policy_block") {
            add_category("policy");
        }
    }

    return categories;
}

uint64_t fnv1a_64(const std::string & input) {
    uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : input) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::string hex_u64(uint64_t value) {
    std::ostringstream oss;
    oss << std::hex;
    oss.width(16);
    oss.fill('0');
    oss << value;
    return oss.str();
}

} // namespace

result evaluate(const std::string & prompt, const metadata & meta) {
    const std::string normalized = normalize_text(prompt);

    const std::vector<std::string> vulnerable_terms = {
        "child", "kid", "baby", "toddler", "infant", "pregnant", "pregnancy", "elderly", "senior",
        "crianca", "menor", "bebe", "gravida", "idoso", "filho", "alergia", "allergy", "allergic", "asma", "asthma"
    };

    const std::vector<std::string> ambiguous_terms = {
        "not sure", "unknown", "mystery", "unlabeled", "no label", "generic label", "unmarked", "random", "unsure", "maybe", "guess", "think",
        "nao sei", "desconhecido", "nao identificado", "mistura", "qualquer", "talvez", "sem rotulo", "rotulo generico", "sem etiqueta", "acho que"
    };

    const std::vector<std::string> instruction_terms = {
        "how to", "step by step", "instructions", "guide", "tutorial", "recipe", "build", "make",
        "como fazer", "como misturar", "passo a passo", "instrucoes", "procedimento", "receita"
    };

    const std::vector<std::string> hazard_terms = {
        "bleach", "ammonia", "chlorine", "acid", "peroxide", "drain cleaner", "cleaner", "solvent", "pesticide",
        "explosive", "bomb", "weapon", "firearm", "poison", "toxic gas",
        "agua sanitaria", "amonia", "cloro", "acido", "peroxido", "vinagre", "saneante", "solvente", "pesticida", "bomba", "arma", "veneno", "gas toxico"
    };

    const std::vector<std::string> mix_terms = {
        "mix", "combine", "blend", "misturar", "misture", "mistura", "combinar"
    };

    const std::vector<std::string> pressure_terms = {
        "sealed", "closed container", "pressurized", "pressure",
        "recipiente fechado", "garrafa fechada", "lata fechada", "pressao"
    };

    const std::vector<std::string> heat_terms = {
        "heat", "ignite", "burn", "flame", "aquec", "esquent", "fogo", "microondas"
    };

    const std::vector<std::string> energy_terms = {
        "energy", "electric", "battery", "power",
        "energia", "eletric", "bateria", "tomada"
    };

    const std::vector<std::string> allergy_terms = {
        "allergy", "allergic", "alergia", "alergico", "alergica"
    };

    const std::vector<std::string> critical_foods = {
        "grapefruit", "sesame"
    };

    bool vulnerable = contains_any(normalized, vulnerable_terms);
    bool ambiguous = contains_any(normalized, ambiguous_terms);
    bool hazard_request = contains_any(normalized, hazard_terms) && contains_any(normalized, instruction_terms);
    bool hazard_context = (contains_any(normalized, hazard_terms) && contains_any(normalized, mix_terms)) ||
                          (contains_any(normalized, pressure_terms) && (contains_any(normalized, heat_terms) || contains_any(normalized, energy_terms)));
    bool allergy_trigger = contains_any(normalized, allergy_terms) && contains_any(normalized, critical_foods);

    for (const auto & field : meta.fields) {
        std::string field_value = normalize_text(field.second);
        if (!vulnerable && contains_any(field_value, vulnerable_terms)) {
            vulnerable = true;
        }
        if (!ambiguous && contains_any(field_value, ambiguous_terms)) {
            ambiguous = true;
        }
    }

    result res;

    if (vulnerable && ambiguous) {
        res.risk_level = 3;
        res.action_taken = action::block;
    } else if (hazard_request) {
        res.risk_level = 3;
        res.action_taken = action::block;
    } else if (hazard_context || allergy_trigger) {
        res.risk_level = 2;
        res.action_taken = action::warn;
    } else {
        res.risk_level = 0;
        res.action_taken = action::allow;
    }

    res.reasons = collect_reasons(vulnerable, ambiguous, hazard_request, hazard_context, allergy_trigger);

    const auto policy_it = meta.fields.find("policy_path");
    if (policy_it != meta.fields.end() && !policy_it->second.empty()) {
        policy_terms policy = load_policy_terms(policy_it->second);
        bool policy_block = contains_any(normalized, policy.block_terms);
        bool policy_warn = contains_any(normalized, policy.warn_terms);
        if (policy_block) {
            res.action_taken = action::block;
            res.risk_level = std::max(res.risk_level, 3);
            res.reasons.push_back("policy_block");
        } else if (policy_warn && res.action_taken == action::allow) {
            res.action_taken = action::warn;
            res.risk_level = std::max(res.risk_level, 2);
            res.reasons.push_back("policy_warn");
        } else if (policy_warn) {
            res.reasons.push_back("policy_warn");
        }
    }

    res.normalized_prompt_digest = hex_u64(fnv1a_64(normalized));
    if (!res.reasons.empty()) {
        res.reason_code = res.reasons.front();
    } else {
        res.reason_code = "none";
    }
    res.categories = collect_categories(res.reasons);
    return res;
}

const std::string & witness_safe() {
    static const std::string message = "AVISA: risco detectado. Resposta apenas em nivel geral.";
    return message;
}

const std::string & witness_block() {
    static const std::string message = "AVISA: BLOQUEADO por risco + ambiguidade.";
    return message;
}

std::string action_to_string(action value) {
    switch (value) {
        case action::allow:
            return "ALLOW";
        case action::warn:
            return "WARN";
        case action::block:
            return "BLOCK";
    }
    return "ALLOW";
}

std::string mode_to_string(mode value) {
    switch (value) {
        case mode::warn:
            return "warn";
        case mode::block:
            return "block";
    }
    return "block";
}

mode mode_from_string(const std::string & value) {
    if (value == "warn") {
        return mode::warn;
    }
    return mode::block;
}

result apply_mode(const result & res, mode guard_mode) {
    result adjusted = res;
    if (guard_mode == mode::warn && adjusted.action_taken == action::block) {
        adjusted.action_taken = action::warn;
    } else if (guard_mode == mode::block && adjusted.action_taken == action::warn) {
        adjusted.action_taken = action::block;
    }
    return adjusted;
}

std::string render_message(const result & res) {
    if (res.action_taken == action::block) {
        return witness_block() + std::string(" Descreva o objetivo em alto nivel ou busque ajuda profissional.");
    }
    if (res.action_taken == action::warn) {
        return witness_safe() + std::string(" Posso responder em alto nivel; considere orientacao profissional.");
    }
    return "";
}

std::string render_metadata_json(const result & res) {
    std::ostringstream oss;
    oss << "{\"guard_result\":{"
        << "\"action_taken\":\"" << action_to_string(res.action_taken) << "\","
        << "\"reason_code\":\"" << res.reason_code << "\","
        << "\"categories\":[";

    for (size_t i = 0; i < res.categories.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << "\"" << res.categories[i] << "\"";
    }

    oss << "],\"normalized_prompt_digest\":\"" << res.normalized_prompt_digest << "\""
        << ",\"risk_level\":" << res.risk_level
        << "},\"message\":\"" << render_message(res) << "\"}";
    return oss.str();
}

} // namespace smart_guard
