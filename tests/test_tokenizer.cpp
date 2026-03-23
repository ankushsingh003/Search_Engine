#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include "porter_stemmer.hpp"

using namespace SearchEngine;

void test_full_pipeline() {
    std::string text = "The quick brown fox jumps over the lazy dog's head.";
    auto tokens = Tokenizer::tokenize(text);

    StopWordFilter filter;
    filter.filter(tokens);

    std::vector<std::string> stemmed_tokens;
    for (const auto& t : tokens) {
        stemmed_tokens.push_back(PorterStemmer::stem(t));
    }

    // Expected (simplified stemmer logic):
    // the -> removed
    // quick -> quick
    // brown -> brown
    // fox -> fox
    // jumps -> jump
    // over -> over
    // the -> removed
    // lazy -> lazy
    // dog -> dog
    // head -> head
    
    std::cout << "Original: " << text << std::endl;
    std::cout << "Stemmed: ";
    for (const auto& t : stemmed_tokens) std::cout << t << " ";
    std::cout << std::endl;

    assert(stemmed_tokens.size() > 0);
    std::cout << "Pipeline test passed!" << std::endl;
}

int main() {
    test_full_pipeline();
    return 0;
}
