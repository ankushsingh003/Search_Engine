#include <iostream>
#include <cassert>
#include "vbyte.hpp"

using namespace SearchEngine;

void test_vbyte() {
    std::vector<uint32_t> test_values = {5, 300, 100000, 0, 127, 128};
    std::vector<uint8_t> encoded;

    for (uint32_t v : test_values) {
        VByteCodec::encode(v, encoded);
    }

    const uint8_t* ptr = encoded.data();
    for (uint32_t v : test_values) {
        uint32_t decoded = VByteCodec::decode(ptr);
        assert(decoded == v);
    }

    std::cout << "VByte test passed!" << std::endl;
}

int main() {
    test_vbyte();
    return 0;
}
