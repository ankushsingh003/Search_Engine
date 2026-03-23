#pragma once

#include <string>
#include <vector>

namespace SearchEngine {

enum class TokenType {
    TERM,
    AND,
    OR,
    NOT,
    LPAREN,
    RPAREN,
    PHRASE
};

struct QueryToken {
    TokenType type;
    std::string value;
};

class QueryTokenizer {
public:
    static std::vector<QueryToken> tokenize(const std::string& query);
};

} // namespace SearchEngine
