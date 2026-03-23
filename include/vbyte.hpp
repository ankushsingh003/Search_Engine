#pragma once

#include <vector>
#include <cstdint>

namespace SearchEngine {

class VByteCodec {
public:
    static void encode(uint32_t value, std::vector<uint8_t>& out);
    static uint32_t decode(const uint8_t*& ptr);
};

} // namespace SearchEngine
