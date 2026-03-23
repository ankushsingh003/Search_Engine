#include <iostream>
#include <cassert>
#include "indexer.hpp"

using namespace SearchEngine;

void test_indexing() {
    InvertedIndex index;
    Indexer indexer(index);

    indexer.index_document(0, "The quick brown fox jumps over the lazy dog.");
    indexer.index_document(1, "The brown dog did not jump.");

    // "quick" (stemmed: quick)
    auto p_quick = index.lookup("quick");
    assert(p_quick != nullptr);
    assert(p_quick->get_postings().size() == 1);
    assert(p_quick->get_postings()[0].doc_id == 0);

    // "brown" (stemmed: brown)
    auto p_brown = index.lookup("brown");
    assert(p_brown != nullptr);
    assert(p_brown->get_postings().size() == 2);

    // "jumps" (stemmed: jump)
    auto p_jump = index.lookup("jump");
    assert(p_jump != nullptr);
    assert(p_jump->get_postings().size() == 2);
    assert(p_jump->get_postings()[0].doc_id == 0);
    assert(p_jump->get_postings()[1].doc_id == 1);

    std::cout << "Indexing in-memory test passed!" << std::endl;

    // Test Persistence
    indexer.save("test.idx");
    
    auto search_results = indexer.search("quick brown jumped");
    // "quick" -> {0}, "brown" -> {0, 1}, "jumped" (stemmed: jump) -> {0, 1}
    // Intersection: {0}
    assert(search_results.size() == 1);
    assert(search_results[0] == 0);

    auto search_results2 = indexer.search("dog");
    // "dog" -> {0, 1}
    assert(search_results2.size() == 2);

    std::cout << "Persistence and Search test passed!" << std::endl;
}

int main() {
    test_indexing();
    return 0;
}
