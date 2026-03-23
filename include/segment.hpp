#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdint>
#include "inverted_index.hpp"
#include "postings_list.hpp"

namespace SearchEngine {

struct SegmentHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t term_count;
};

class SegmentWriter {
public:
    static void write(const std::string& path, const InvertedIndex& index);
};

class SegmentReader {
public:
    virtual ~SegmentReader() = default;
    virtual std::vector<Posting> lookup(const std::string& term) const = 0;
};

class FileSegmentReader : public SegmentReader {
public:
    explicit FileSegmentReader(const std::string& path);
    std::vector<Posting> lookup(const std::string& term) const override;
private:
    std::string path_;
    std::map<std::string, std::pair<uint64_t, uint32_t> > term_index_;
    void load_index();
};

class MmapSegmentReader : public SegmentReader {
public:
    explicit MmapSegmentReader(const std::string& path);
    ~MmapSegmentReader();
    std::vector<Posting> lookup(const std::string& term) const override;
private:
    void* file_handle_;
    void* mapping_handle_;
    const uint8_t* data_;
    uint64_t size_;
    std::map<std::string, std::pair<uint64_t, uint32_t> > term_index_;
    void load_index();
};

} // namespace SearchEngine
