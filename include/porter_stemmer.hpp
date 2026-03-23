#pragma once

#include <string>

namespace SearchEngine {

class PorterStemmer {
public:
    static std::string stem(const std::string& word);
};

} // namespace SearchEngine
