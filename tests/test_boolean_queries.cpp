#include <iostream>
#include <cassert>
#include "indexer.hpp"

using namespace SearchEngine;

void test_boolean_queries() {
    InvertedIndex index;
    Indexer indexer(index);

    indexer.index_document(0, "The quick brown fox");
    indexer.index_document(1, "The lazy dog");
    indexer.index_document(2, "The quick brown dog");

    // OR test
    auto results_or = indexer.search("fox OR dog");
    // fox -> {0}, dog -> {1, 2}. OR -> {0, 1, 2}
    assert(results_or.size() == 3);

    // AND NOT test
    auto results_not = indexer.search("dog NOT brown");
    // dog -> {1, 2}, brown -> {0, 2}. NOT brown -> {1} (from universe {0, 1, 2}).
    // dog AND NOT brown -> {1}
    assert(results_not.size() == 1);
    assert(results_not[0].doc_id == 1);

    // Parentheses test
    auto results_paren = indexer.search("(fox OR dog) AND brown");
    // (fox OR dog) -> {0, 1, 2}, brown -> {0, 2}. AND -> {0, 2}
    assert(results_paren.size() == 2);

    std::cout << "Boolean queries test passed!" << std::endl;
}

int main() {
    test_boolean_queries();
    return 0;
}
