#include "postings_list.hpp"

namespace SearchEngine {

void PostingsList::add_occurrence(DocID doc_id, Position pos) {
    if (postings.empty() || postings.back().doc_id != doc_id) {
        postings.push_back({doc_id, 1, {pos}});
    } else {
        postings.back().term_freq++;
        postings.back().positions.push_back(pos);
    }
}

} // namespace SearchEngine
