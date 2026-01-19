#pragma once

#include <string>

namespace smart_guard {

// Table-driven synonym normalizer for critical terms.
// Normalization is conservative: only terms in the explicit table are mapped.
std::string normalize_text(const std::string & text);

// Returns the canonical term for a normalized token, or the token as-is.
std::string normalize_token(const std::string & token);

} // namespace smart_guard
