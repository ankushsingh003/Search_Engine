#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace SearchEngine {

class StopWordFilter {
public:
    StopWordFilter();
    bool isStopWord(const std::string& word) const;
    void filter(std::vector<std::string>& tokens) const;

private:
    std::unordered_set<std::string> stop_words;
};

} // namespace SearchEngine
