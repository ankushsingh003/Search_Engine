#include "query_executor.hpp"
#include <stack>
#include <algorithm>
#include <iterator>

namespace SearchEngine {

std::vector<DocID> QueryExecutor::execute(const std::string& query, Indexer& indexer) {
    auto postfix = QueryParser::parse(query);
    if (postfix.empty()) return {};

    std::stack<std::set<DocID> > stack;

    for (size_t i = 0; i < postfix.size(); ++i) {
        const auto& token = postfix[i];

        if (token.type == TokenType::TERM) {
            std::vector<DocID> results = indexer.search_term(token.value);
            stack.push(std::set<DocID>(results.begin(), results.end()));
        } else if (token.type == TokenType::AND) {
            if (stack.size() < 2) continue;
            std::set<DocID> s2 = stack.top(); stack.pop();
            std::set<DocID> s1 = stack.top(); stack.pop();
            std::set<DocID> result;
            std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(result, result.begin()));
            stack.push(result);
        } else if (token.type == TokenType::OR) {
            if (stack.size() < 2) continue;
            std::set<DocID> s2 = stack.top(); stack.pop();
            std::set<DocID> s1 = stack.top(); stack.pop();
            std::set<DocID> result = s1;
            result.insert(s2.begin(), s2.end());
            stack.push(result);
        } else if (token.type == TokenType::NOT) {
            if (stack.empty()) continue;
            std::set<DocID> s = stack.top(); stack.pop();
            // Boolean NOT without a left operand is tricky.
            // If it's "A AND NOT B", it works. If it's just "NOT B", we need universe.
            // For now, let's treat it as "Universe - S".
            // We'll get universe from indexer.
            std::set<DocID> universe = indexer.get_all_doc_ids();
            std::set<DocID> result;
            std::set_difference(universe.begin(), universe.end(), s.begin(), s.end(), std::inserter(result, result.begin()));
            stack.push(result);
        }
    }

    if (stack.empty()) return {};
    std::set<DocID> final_set = stack.top();
    return std::vector<DocID>(final_set.begin(), final_set.end());
}

} // namespace SearchEngine
