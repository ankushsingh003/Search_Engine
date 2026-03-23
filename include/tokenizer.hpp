#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace SearchEngine {

class Tokenizer {
public:
    static std::vector<std::string> tokenize(std::string_view text);
};

} // namespace SearchEngine
