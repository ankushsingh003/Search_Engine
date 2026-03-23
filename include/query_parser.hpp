#pragma once

#include "query_tokenizer.hpp"
#include <vector>
#include <stack>

namespace SearchEngine {

class QueryParser {
public:
    // Transforms infix tokens to postfix (RPN)
    static std::vector<QueryToken> parse(const std::string& query);

private:
    static int precedence(TokenType type);
};

} // namespace SearchEngine
