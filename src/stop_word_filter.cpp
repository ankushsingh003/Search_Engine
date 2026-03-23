#include "stop_word_filter.hpp"
#include <algorithm>

namespace SearchEngine {

StopWordFilter::StopWordFilter() {
    // Basic set of English stop words
    stop_words = {
        "a", "an", "and", "are", "as", "at", "be", "but", "by",
        "for", "if", "in", "into", "is", "it", "no", "not", "of",
        "on", "or", "such", "that", "the", "their", "then", "there",
        "these", "they", "this", "to", "was", "will", "with"
    };
}

bool StopWordFilter::isStopWord(const std::string& word) const {
    return stop_words.find(word) != stop_words.end();
}

void StopWordFilter::filter(std::vector<std::string>& tokens) const {
    tokens.erase(
        std::remove_if(tokens.begin(), tokens.end(),
                       [this](const std::string& t) { return isStopWord(t); }),
        tokens.end()
    );
}

} // namespace SearchEngine
