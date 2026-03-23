#pragma once

#include <string>
#include <vector>
#include <set>

namespace SearchEngine {

class SnippetGenerator {
public:
    static std::string generate(const std::string& document, const std::set<std::string>& query_terms, size_t window_size = 20);
};

} // namespace SearchEngine
