#pragma once

#include "query_parser.hpp"
#include "indexer.hpp"
#include <set>

namespace SearchEngine {

class QueryExecutor {
public:
    static std::vector<DocID> execute(const std::string& query, Indexer& indexer);
};

} // namespace SearchEngine
