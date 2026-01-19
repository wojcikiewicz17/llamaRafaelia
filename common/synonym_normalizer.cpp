#include "synonym_normalizer.h"

#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace smart_guard {
namespace {

std::string normalize_ascii(const std::string & input) {
    static const std::vector<std::pair<std::string, char>> map = {
        {"á", 'a'}, {"à", 'a'}, {"â", 'a'}, {"ã", 'a'}, {"ä", 'a'},
        {"Á", 'a'}, {"À", 'a'}, {"Â", 'a'}, {"Ã", 'a'}, {"Ä", 'a'},
        {"é", 'e'}, {"ê", 'e'}, {"è", 'e'}, {"ë", 'e'},
        {"É", 'e'}, {"Ê", 'e'}, {"È", 'e'}, {"Ë", 'e'},
        {"í", 'i'}, {"î", 'i'}, {"ì", 'i'}, {"ï", 'i'},
        {"Í", 'i'}, {"Î", 'i'}, {"Ì", 'i'}, {"Ï", 'i'},
        {"ó", 'o'}, {"ô", 'o'}, {"ò", 'o'}, {"õ", 'o'}, {"ö", 'o'},
        {"Ó", 'o'}, {"Ô", 'o'}, {"Ò", 'o'}, {"Õ", 'o'}, {"Ö", 'o'},
        {"ú", 'u'}, {"û", 'u'}, {"ù", 'u'}, {"ü", 'u'},
        {"Ú", 'u'}, {"Û", 'u'}, {"Ù", 'u'}, {"Ü", 'u'},
        {"ç", 'c'}, {"Ç", 'c'},
    };

    std::string out;
    out.reserve(input.size());
    for (size_t i = 0; i < input.size();) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char>(std::tolower(c)));
            ++i;
            continue;
        }
        bool matched = false;
        for (const auto & entry : map) {
            const std::string & key = entry.first;
            if (input.compare(i, key.size(), key) == 0) {
                out.push_back(entry.second);
                i += key.size();
                matched = true;
                break;
            }
        }
        if (!matched) {
            ++i;
        }
    }
    return out;
}

const std::unordered_map<std::string, std::string> & synonym_table() {
    static const std::unordered_map<std::string, std::string> table = {
        {"toranja", "grapefruit"},
        {"pomelo", "grapefruit"},
        {"grapefruit", "grapefruit"},
        {"gergelim", "sesame"},
        {"sesame", "sesame"},
        {"sesamo", "sesame"},
    };
    return table;
}

std::vector<std::string> tokenize_simple(const std::string & text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(c);
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

} // namespace

std::string normalize_token(const std::string & token) {
    std::string normalized = normalize_ascii(token);
    auto it = synonym_table().find(normalized);
    if (it != synonym_table().end()) {
        return it->second;
    }
    return normalized;
}

std::string normalize_text(const std::string & text) {
    std::string ascii = normalize_ascii(text);
    std::vector<std::string> tokens = tokenize_simple(ascii);
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            out.push_back(' ');
        }
        out += normalize_token(tokens[i]);
    }
    return out;
}

} // namespace smart_guard
