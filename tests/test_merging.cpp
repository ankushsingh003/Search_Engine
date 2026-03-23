#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "indexer.hpp"
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include "porter_stemmer.hpp"
#include "query_executor.hpp"

using namespace SearchEngine;

int main() {
    InvertedIndex idx;
    Indexer indexer(idx);

    // 1. Index and Save Segment 1
    indexer.index_document(1, "The quick brown fox");
    indexer.index_document(2, "Jumps over the lazy dog");
    indexer.save("test_seg1.bin");
    assert(indexer.get_segment_count() == 1);

    // 2. Index and Save Segment 2
    indexer.index_document(3, "The fox is quick");
    indexer.save("test_seg2.bin");
    assert(indexer.get_segment_count() == 2);

    // 3. Delete doc 1 and add doc 4 in memory
    indexer.delete_document(1);
    indexer.index_document(4, "A lazy fox");
    
    std::cout << "Before merge, segments: " << indexer.get_segment_count() << std::endl;

    // 4. Merge
    indexer.merge_segments("test_merged.bin");
    std::cout << "After merge, segments: " << indexer.get_segment_count() << std::endl;
    assert(indexer.get_segment_count() == 1);

    // 5. Verify results
    // Search for "fox" (should be in 2, 3, 4 - but NOT 1)
    auto results = indexer.search("fox");
    std::cout << "Results for 'fox': " << results.size() << std::endl;
    for (const auto& r : results) {
        std::cout << "  Found external DocID: " << r.doc_id << std::endl;
    }
    
    bool found1 = false;
    for (const auto& r : results) {
        if (r.doc_id == 1) found1 = true;
    }
    assert(!found1);
    assert(results.size() >= 2); // doc 3, 4 (and 2 doesn't have fox)

    // Search for "lazy" (should be in 2, 4)
    auto results2 = indexer.search("lazy");
    std::cout << "Results for 'lazy': " << results2.size() << std::endl;
    assert(results2.size() == 2);

    std::cout << "Segment merging tests passed!" << std::endl;

    // Cleanup
    std::remove("test_seg1.bin");
    std::remove("test_seg2.bin");
    std::remove("test_merged.bin");

    return 0;
}
