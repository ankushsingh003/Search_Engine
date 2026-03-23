#pragma once

#include <vector>
#include <cstdint>

namespace SearchEngine {

using DocID = uint32_t;
using Position = uint32_t;

struct Posting {
    DocID doc_id;
    uint32_t term_freq;
    std::vector<Position> positions;
};

class PostingsList {
public:
    void add_occurrence(DocID doc_id, Position pos);
    const std::vector<Posting>& get_postings() const { return postings; }

private:
    std::vector<Posting> postings;
};

} // namespace SearchEngine
