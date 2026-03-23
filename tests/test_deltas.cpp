#include <iostream>
#include <cassert>
#include "indexer.hpp"

using namespace SearchEngine;

void test_deltas() {
    InvertedIndex index;
    Indexer indexer(index);

    indexer.index_document(0, "The quick brown fox");
    indexer.index_document(1, "The lazy dog");
    
    // Initial search
    auto res1 = indexer.search("fox");
    assert(res1.size() == 1);
    assert(res1[0].doc_id == 0);

    // Delete doc 0
    indexer.delete_document(0);
    auto res2 = indexer.search("fox");
    assert(res2.size() == 0);

    // Update doc 1 content
    // Old content: "The lazy dog"
    // New content: "The active cat"
    indexer.update_document(1, "The active cat");
    
    auto res3 = indexer.search("dog");
    assert(res3.size() == 0); // doc 1 no longer has "dog"
    
    auto res4 = indexer.search("cat");
    assert(res4.size() == 1);
    assert(res4[0].doc_id == 1);

    // Verify document count
    // Original total docs was 2 (0 and 1).
    // Now doc 0 is deleted, so total docs should be 1 (doc 1).
    assert(indexer.get_total_docs() == 1);

    std::cout << "Delta updates tests passed!" << std::endl;
}

int main() {
    test_deltas();
    return 0;
}
