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
        } else if (token.type == TokenType::PHRASE) {
            std::set<DocID> result = evaluate_phrase(token.value, indexer);
            stack.push(result);
        }
    }

    if (stack.empty()) return {};
    std::set<DocID> final_set = stack.top();
    return std::vector<DocID>(final_set.begin(), final_set.end());
}

std::set<DocID> QueryExecutor::evaluate_phrase(const std::string& phrase, Indexer& indexer) {
    auto terms = Tokenizer::tokenize(phrase);
    if (terms.empty()) return {};

    // Get all postings for each term
    std::vector<std::map<DocID, Posting> > term_postings;
    for (const auto& term : terms) {
        term_postings.push_back(indexer.get_term_postings(term));
    }

    if (term_postings.empty()) return {};

    // Start with doc IDs from the first term
    std::set<DocID> candidates;
    for (auto const& pair : term_postings[0]) {
        candidates.insert(pair.first);
    }

    // Intersect with doc IDs from other terms
    for (size_t i = 1; i < term_postings.size(); ++i) {
        std::set<DocID> current_docs;
        for (auto const& pair : term_postings[i]) {
            if (candidates.count(pair.first)) {
                current_docs.insert(pair.first);
            }
        }
        candidates = current_docs;
    }

    // For each candidate doc, verify consecutive positions
    std::set<DocID> final_results;
    for (DocID doc_id : candidates) {
        // positions[term_index] -> list of positions
        const auto& first_term_positions = term_postings[0][doc_id].positions;
        
        for (Position start_pos : first_term_positions) {
            bool found = true;
            for (size_t i = 1; i < term_postings.size(); ++i) {
                const auto& next_term_positions = term_postings[i][doc_id].positions;
                Position target_pos = start_pos + static_cast<Position>(i);
                
                if (!std::binary_search(next_term_positions.begin(), next_term_positions.end(), target_pos)) {
                    found = false;
                    break;
                }
            }
            if (found) {
                final_results.insert(doc_id);
                break;
            }
        }
    }

    return final_results;
}

} // namespace SearchEngine
