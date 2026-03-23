#include <iostream>
#include <cassert>
#include "postings_list.hpp"

using namespace SearchEngine;

void test_postings_serialization() {
    PostingsList pl;
    pl.add_occurrence(10, 5);
    pl.add_occurrence(10, 15);
    pl.add_occurrence(100, 2);
    pl.add_occurrence(100, 50);
    pl.add_occurrence(100, 200);

    std::vector<uint8_t> encoded;
    pl.serialize(encoded);

    const uint8_t* ptr = encoded.data();
    PostingsList decoded_pl = PostingsList::deserialize(ptr, 2);

    auto& p1 = decoded_pl.get_postings()[0];
    assert(p1.doc_id == 10);
    assert(p1.term_freq == 2);
    assert(p1.positions[0] == 5);
    assert(p1.positions[1] == 15);

    auto& p2 = decoded_pl.get_postings()[1];
    assert(p2.doc_id == 100);
    assert(p2.term_freq == 3);
    assert(p2.positions[0] == 2);
    assert(p2.positions[1] == 50);
    assert(p2.positions[2] == 200);

    std::cout << "Postings serialization test passed!" << std::endl;
}

int main() {
    test_postings_serialization();
    return 0;
}
