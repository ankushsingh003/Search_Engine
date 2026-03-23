#include "inverted_index.hpp"

namespace SearchEngine {

void InvertedIndex::add_term(const std::string& term, DocID doc_id, Position pos) {
    index[term].add_posting(doc_id, 1, {pos});
}

void InvertedIndex::add_postings_list(const std::string& term, const PostingsList& pl) {
    index[term] = pl;
}

const PostingsList* InvertedIndex::lookup(const std::string& term) const {
    auto it = index.find(term);
    if (it != index.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace SearchEngine
