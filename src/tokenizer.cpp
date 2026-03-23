#include "tokenizer.hpp"
#include <algorithm>
#include <cctype>

namespace SearchEngine {

std::vector<std::string> Tokenizer::tokenize(std::string_view text) {
    std::vector<std::string> tokens;
    std::string current_token;

    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current_token += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }

    return tokens;
}

} // namespace SearchEngine
