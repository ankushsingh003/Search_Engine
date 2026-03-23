#pragma once

#include <string>
#include <vector>

namespace SearchEngine {

class Tokenizer {
public:
    static std::vector<std::string> tokenize(const std::string& content);
};

} // namespace SearchEngine
