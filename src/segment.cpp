#include "segment.hpp"
#include <iostream>

namespace SearchEngine {

void SegmentWriter::write(const std::string& path, const InvertedIndex& index) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return;

    // Header: Term count (placeholder/stage 1)
    // For now, let's just write the data in a simple format:
    // [num_terms]
    // [term_len][term][offset][num_postings]
    // ...
    // [postings_data]

    // This is a simplified version to get persistence working.
    // In Week 7, we'll refine this for mmap.
}

} // namespace SearchEngine
