#include "inverted_index.hpp"

namespace SearchEngine {

void InvertedIndex::add_term(const std::string& term, DocID doc_id, Position pos) {
    index[term].add_occurrence(doc_id, pos);
}

const PostingsList* InvertedIndex::lookup(const std::string& term) const {
    auto it = index.find(term);
    if (it != index.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace SearchEngine
