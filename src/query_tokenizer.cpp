#include "query_tokenizer.hpp"
#include <cctype>
#include <algorithm>

namespace SearchEngine {

std::vector<QueryToken> QueryTokenizer::tokenize(const std::string& query) {
    std::vector<QueryToken> tokens;
    std::string current_term;

    for (size_t i = 0; i < query.length(); ++i) {
        char c = query[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current_term.empty()) {
                // Check if term is an operator
                std::string upper = current_term;
                for (size_t j = 0; j < upper.length(); ++j) upper[j] = std::toupper(static_cast<unsigned char>(upper[j]));
                
                if (upper == "AND") tokens.push_back({TokenType::AND, ""});
                else if (upper == "OR") tokens.push_back({TokenType::OR, ""});
                else if (upper == "NOT") tokens.push_back({TokenType::NOT, ""});
                else tokens.push_back({TokenType::TERM, current_term});
                
                current_term.clear();
            }
        } else if (c == '(') {
            if (!current_term.empty()) {
                tokens.push_back({TokenType::TERM, current_term});
                current_term.clear();
            }
            tokens.push_back({TokenType::LPAREN, ""});
        } else if (c == ')') {
            if (!current_term.empty()) {
                tokens.push_back({TokenType::TERM, current_term});
                current_term.clear();
            }
            tokens.push_back({TokenType::RPAREN, ""});
        } else if (c == '\"') {
            if (!current_term.empty()) {
                tokens.push_back({TokenType::TERM, current_term});
                current_term.clear();
            }
            // Scan until next quote
            std::string phrase;
            i++;
            while (i < query.length() && query[i] != '\"') {
                phrase += query[i];
                i++;
            }
            tokens.push_back({TokenType::PHRASE, phrase});
        } else {
            current_term += c;
        }
    }

    if (!current_term.empty()) {
        std::string upper = current_term;
        for (size_t j = 0; j < upper.length(); ++j) upper[j] = std::toupper(static_cast<unsigned char>(upper[j]));
        
        if (upper == "AND") tokens.push_back({TokenType::AND, ""});
        else if (upper == "OR") tokens.push_back({TokenType::OR, ""});
        else if (upper == "NOT") tokens.push_back({TokenType::NOT, ""});
        else tokens.push_back({TokenType::TERM, current_term});
    }

    return tokens;
}

} // namespace SearchEngine
