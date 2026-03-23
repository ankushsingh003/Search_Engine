#pragma once
#include "postings_list.hpp"
#include <vector>
#include <string>

namespace SearchEngine {

class Indexer;

struct RankedResult {
    DocID doc_id;
    double score;
};

class Ranker {
public:
    static std::vector<RankedResult> rank(const std::vector<DocID>& docs, const std::string& query, Indexer& indexer);
};

} // namespace SearchEngine
