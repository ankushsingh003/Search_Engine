#include <iostream>
#include <cassert>
#include "indexer.hpp"
#include "query_executor.hpp"

using namespace SearchEngine;

void test_phrases() {
    InvertedIndex index;
    Indexer indexer(index);

    indexer.index_document(0, "The quick brown fox jumps over the lazy dog");
    indexer.index_document(1, "The quick brown dog jumps over the lazy fox");
    indexer.index_document(2, "A quick brown fox");

    // Test simple phrase
    auto res1 = QueryExecutor::execute("\"quick brown fox\"", indexer);
    std::cout << "res1 size: " << res1.size() << std::endl;
    for (auto d : res1) std::cout << " d: " << d << std::endl;
    assert(res1.size() == 2); // docs 0 and 2
    
    // Test phrase that exists in one doc but not another
    auto res2 = QueryExecutor::execute("\"brown dog jumps\"", indexer);
    std::cout << "res2 size: " << res2.size() << std::endl;
    assert(res2.size() == 1);
    assert(res2[0] == 1);

    // Test phrase with non-consecutive terms
    auto res3 = QueryExecutor::execute("\"quick jumps\"", indexer);
    std::cout << "res3 size: " << res3.size() << std::endl;
    assert(res3.size() == 0);

    // Combined boolean and phrase
    auto res4 = QueryExecutor::execute("\"quick brown\" AND fox", indexer);
    assert(res4.size() == 3); // docs 0, 1, 2 all have "quick brown" and "fox"

    auto res5 = QueryExecutor::execute("\"quick brown\" AND dog", indexer);
    assert(res5.size() == 2); // docs 0 and 1

    std::cout << "Phrase queries tests passed!" << std::endl;
}

int main() {
    test_phrases();
    return 0;
}
