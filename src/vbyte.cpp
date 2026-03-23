#include "vbyte.hpp"

namespace SearchEngine {

void VByteCodec::encode(uint32_t value, std::vector<uint8_t>& out) {
    while (value > 127) {
        out.push_back(static_cast<uint8_t>(value & 0x7F) | 0x80);
        value >>= 7;
    }
    out.push_back(static_cast<uint8_t>(value & 0x7F));
}

uint32_t VByteCodec::decode(const uint8_t*& ptr) {
    uint32_t result = 0;
    uint32_t shift = 0;
    while (*ptr & 0x80) {
        result |= static_cast<uint32_t>(*ptr++ & 0x7F) << shift;
        shift += 7;
    }
    result |= static_cast<uint32_t>(*ptr++ & 0x7F) << shift;
    return result;
}

} // namespace SearchEngine
