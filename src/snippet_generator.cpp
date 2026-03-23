#include "snippet_generator.hpp"
#include "tokenizer.hpp"
#include "porter_stemmer.hpp"
#include <sstream>
#include <algorithm>
#include <vector>

namespace SearchEngine {

std::string SnippetGenerator::generate(const std::string& document, const std::set<std::string>& query_terms, size_t window_size) {
    if (document.empty() || query_terms.empty()) return "";

    // Stem query terms for matching
    std::set<std::string> stemmed_queries;
    for (const auto& term : query_terms) {
        stemmed_queries.insert(PorterStemmer::stem(term));
    }

    // Tokenize document but keep original words for display
    // We need a way to get word offsets. Our Tokenizer doesn't do that yet.
    // Let's do a simple whitespace/punctuation split for now to find matching positions.
    
    std::vector<std::string> words;
    std::string current_word;
    for (char c : document) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current_word += c;
        } else {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
            words.push_back(std::string(1, c)); // Keep punctuation/spaces for the snippet
        }
    }
    if (!current_word.empty()) words.push_back(current_word);

    // Find the first matching stemmed token
    int match_index = -1;
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i].length() > 0 && std::isalnum(static_cast<unsigned char>(words[i][0]))) {
            std::string stemmed = PorterStemmer::stem(words[i]);
            if (stemmed_queries.count(stemmed)) {
                match_index = static_cast<int>(i);
                break;
            }
        }
    }

    if (match_index == -1) {
        // Just return the beginning
        match_index = 0;
    }

    // Determine window
    int start = std::max(0, match_index - static_cast<int>(window_size / 2));
    int end = std::min(static_cast<int>(words.size()), start + static_cast<int>(window_size));
    
    // Adjust start if end hit the boundary
    start = std::max(0, end - static_cast<int>(window_size));

    std::stringstream ss;
    if (start > 0) ss << "... ";
    
    for (int i = start; i < end; ++i) {
        std::string raw_word = words[i];
        bool is_match = false;
        
        if (!raw_word.empty() && std::isalnum(static_cast<unsigned char>(raw_word[0]))) {
            std::string stemmed = PorterStemmer::stem(raw_word);
            if (stemmed_queries.count(stemmed)) {
                is_match = true;
            }
        }

        if (is_match) ss << "**" << raw_word << "**";
        else ss << raw_word;
    }

    if (end < static_cast<int>(words.size())) ss << " ...";

    return ss.str();
}

} // namespace SearchEngine
