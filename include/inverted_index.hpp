#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "postings_list.hpp"

namespace SearchEngine {

class InvertedIndex {
public:
    void add_term(const std::string& term, DocID doc_id, Position pos);
    const PostingsList* lookup(const std::string& term) const;
    size_t size() const { return index.size(); }
    const std::unordered_map<std::string, PostingsList>& get_index() const { return index; }

private:
    std::unordered_map<std::string, PostingsList> index;
};

} // namespace SearchEngine
