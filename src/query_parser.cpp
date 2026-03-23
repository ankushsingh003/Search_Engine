#include "query_parser.hpp"
#include <stdexcept>

namespace SearchEngine {

int QueryParser::precedence(TokenType type) {
    switch (type) {
        case TokenType::NOT: return 3;
        case TokenType::AND: return 2;
        case TokenType::OR: return 1;
        default: return 0;
    }
}

std::vector<QueryToken> QueryParser::parse(const std::string& query) {
    auto infix = QueryTokenizer::tokenize(query);
    std::vector<QueryToken> postfix;
    std::stack<QueryToken> ops;

    // Add implicit ANDs between terms or (term andterm)
    std::vector<QueryToken> infix_with_and;
    for (size_t i = 0; i < infix.size(); ++i) {
        if (i > 0) {
            bool prev_is_operand = (infix[i-1].type == TokenType::TERM || infix[i-1].type == TokenType::RPAREN);
            bool curr_is_operand = (infix[i].type == TokenType::TERM || infix[i].type == TokenType::LPAREN || infix[i].type == TokenType::NOT);
            
            if (prev_is_operand && curr_is_operand) {
                infix_with_and.push_back({TokenType::AND, ""});
            }
        }
        infix_with_and.push_back(infix[i]);
    }

    for (size_t i = 0; i < infix_with_and.size(); ++i) {
        const auto& token = infix_with_and[i];
        
        if (token.type == TokenType::TERM) {
            postfix.push_back(token);
        } else if (token.type == TokenType::LPAREN) {
            ops.push(token);
        } else if (token.type == TokenType::RPAREN) {
            while (!ops.empty() && ops.top().type != TokenType::LPAREN) {
                postfix.push_back(ops.top());
                ops.pop();
            }
            if (!ops.empty()) ops.pop(); // remove LPAREN
        } else {
            // Operator
            while (!ops.empty() && ops.top().type != TokenType::LPAREN && 
                   precedence(ops.top().type) >= precedence(token.type)) {
                postfix.push_back(ops.top());
                ops.pop();
            }
            ops.push(token);
        }
    }

    while (!ops.empty()) {
        postfix.push_back(ops.top());
        ops.pop();
    }

    return postfix;
}

} // namespace SearchEngine
