#include <iostream>
#include <cassert>
#include "indexer.hpp"

using namespace SearchEngine;

void test_ranking() {
    InvertedIndex index;
    Indexer indexer(index);

    // Doc 0: "apple apple apple" (High TF for apple)
    // Doc 1: "apple orange" (Low TF for apple, contains orange)
    // Doc 2: "orange banana" (Contains orange, rare term banana)
    
    indexer.index_document(0, "apple apple apple");
    indexer.index_document(1, "apple orange");
    indexer.index_document(2, "orange banana");

    // Test 1: Search for "apple"
    // Doc 0 should be first due to higher TF.
    auto results_apple = indexer.search("apple");
    assert(results_apple.size() == 2);
    assert(results_apple[0].doc_id == 0);
    assert(results_apple[1].doc_id == 1);
    assert(results_apple[0].score > results_apple[1].score);

    // Test 2: Search for "orange"
    // Documents containing orange should have similar scores (both TF=1).
    auto results_orange = indexer.search("orange");
    assert(results_orange.size() == 2);

    // Test 3: Search for "banana"
    // Banana is rarer (DF=1) compared to apple (DF=2) or orange (DF=2).
    // In a query "apple banana", banana should contribute more to the score if N was larger, 
    // but with N=3, IDF(banana) = log(3/1) = 0.47, IDF(apple) = log(3/2) = 0.17.
    auto results_multi = indexer.search("apple banana");
    // Doc 2 has banana, Doc 0 has apple (TF=3), Doc 1 has apple (TF=1).
    // Score(Doc 2) = 1 * log(3/1) = 0.477
    // Score(Doc 0) = 3 * log(3/2) = 3 * 0.176 = 0.528
    // So Doc 0 might still be higher due to high TF.
    
    std::cout << "Ranking test results for 'apple banana':" << std::endl;
    for (size_t i = 0; i < results_multi.size(); ++i) {
        std::cout << "Doc " << results_multi[i].doc_id << ": " << results_multi[i].score << std::endl;
    }

    std::cout << "Ranking tests passed!" << std::endl;
}

int main() {
    test_ranking();
    return 0;
}
