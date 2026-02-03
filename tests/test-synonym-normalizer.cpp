#include "synonym_normalizer.h"

#include <cassert>
#include <string>

int main() {
    const std::string normalized = smart_guard::normalize_text("Toranja e Gérgelim");
    assert(normalized == "grapefruit e sesame");

    const std::string normalized_ascii = smart_guard::normalize_text("Gergelim");
    assert(normalized_ascii == "sesame");

    return 0;
}
