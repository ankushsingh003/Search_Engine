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

    std::cout << "Indexing test passed!" << std::endl;
}

int main() {
    test_indexing();
    return 0;
}
