#include "porter_stemmer.hpp"
#include <iostream>

namespace SearchEngine {

std::string PorterStemmer::stem(const std::string& word) {
    if (word.length() <= 2) return word;

    std::string s = word;

    // Very simplified Porter-style rules for demonstration
    // In a real project, we would use the full Porter or Snowball library.

    // Rule 1: s, es, ed, ing
    if (s.ends_with("sses")) {
        s = s.substr(0, s.length() - 2);
    } else if (s.ends_with("ies")) {
        s = s.substr(0, s.length() - 2);
    } else if (s.ends_with("ss")) {
        // do nothing
    } else if (s.ends_with("s")) {
        s = s.substr(0, s.length() - 1);
    }

    if (s.ends_with("eed")) {
        if (s.length() > 4) s = s.substr(0, s.length() - 1);
    } else if (s.ends_with("ed")) {
        s = s.substr(0, s.length() - 2);
    } else if (s.ends_with("ing")) {
        s = s.substr(0, s.length() - 3);
    }

    return s;
}

} // namespace SearchEngine
