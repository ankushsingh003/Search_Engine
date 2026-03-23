#include "porter_stemmer.hpp"
#include <iostream>

namespace SearchEngine {

// Helper function for C++11 compatibility
static bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string PorterStemmer::stem(const std::string& word) {
    if (word.length() <= 2) return word;

    std::string s = word;

    // Very simplified Porter-style rules for demonstration
    // Rule 1: s, es, ed, ing
    if (ends_with(s, "sses")) {
        s = s.substr(0, s.length() - 2);
    } else if (ends_with(s, "ies")) {
        s = s.substr(0, s.length() - 2);
    } else if (ends_with(s, "ss")) {
        // do nothing
    } else if (ends_with(s, "s")) {
        s = s.substr(0, s.length() - 1);
    }

    if (ends_with(s, "eed")) {
        if (s.length() > 4) s = s.substr(0, s.length() - 1);
    } else if (ends_with(s, "ed")) {
        s = s.substr(0, s.length() - 2);
    } else if (ends_with(s, "ing")) {
        s = s.substr(0, s.length() - 3);
    }

    return s;
}

} // namespace SearchEngine
