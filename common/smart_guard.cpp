#include "smart_guard.h"
#include "synonym_normalizer.h"

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

std::string render_message(const result & res) {
    if (res.action_taken == action::block) {
        return witness_block() + std::string(" Descreva o objetivo em alto nivel ou busque ajuda profissional.");
    }
    if (res.action_taken == action::warn) {
        return witness_safe() + std::string(" Posso responder em alto nivel; considere orientacao profissional.");
    }
    return "";
}

} // namespace smart_guard
