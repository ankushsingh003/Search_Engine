#include "ranker.hpp"
#include "indexer.hpp"
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include <cmath>
#include <algorithm>
#include <map>

namespace SearchEngine {

bool compareRankedResults(const RankedResult& a, const RankedResult& b) {
    return a.score > b.score;
}

std::vector<RankedResult> Ranker::rank(const std::vector<DocID>& docs, const std::string& query, Indexer& indexer) {
    auto tokens = Tokenizer::tokenize(query);
    StopWordFilter filter;
    filter.filter(tokens);
    
    if (tokens.empty() || docs.empty()) return {};

    std::vector<RankedResult> ranked_results;
    std::map<DocID, double> scores;
    size_t N = indexer.get_total_docs();

    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& term = tokens[i];
        std::map<DocID, uint32_t> term_tfs = indexer.get_term_tfs(term);
        size_t df = term_tfs.size();
        
        if (df == 0) continue;

        double idf = std::log10(static_cast<double>(N) / df);

        for (size_t j = 0; j < docs.size(); ++j) {
            DocID doc_id = docs[j];
            if (term_tfs.count(doc_id)) {
                scores[doc_id] += static_cast<double>(term_tfs[doc_id]) * idf;
            }
        }
    }

    for (std::map<DocID, double>::iterator it = scores.begin(); it != scores.end(); ++it) {
        RankedResult res;
        res.doc_id = it->first;
        res.score = it->second;
        ranked_results.push_back(res);
    }

    std::sort(ranked_results.begin(), ranked_results.end(), compareRankedResults);
    return ranked_results;
}

} // namespace SearchEngine
