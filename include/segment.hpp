#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include "inverted_index.hpp"

namespace SearchEngine {

class SegmentWriter {
public:
    static void write(const std::string& path, const InvertedIndex& index);
};

// Map-based reader for simplicity in Stage 1, will move to mmap in Stage 2
class SegmentReader {
public:
    virtual ~SegmentReader() = default;
    virtual const PostingsList* lookup(const std::string& term) const = 0;
};

} // namespace SearchEngine
