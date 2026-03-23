#include "segment.hpp"
#include "vbyte.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace SearchEngine {

void SegmentWriter::write(const std::string& path, const InvertedIndex& index) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return;

    const auto& map = index.get_index();
    SegmentHeader header = {0x53455243, 1, static_cast<uint64_t>(map.size())};
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Term Index construction
    // We'll write the postings first, then the term index at the end or vice-versa.
    // Let's write Term Index first, then Postings.
    // To do that, we need to know the offsets of postings.
    
    std::vector<std::pair<std::string, std::vector<uint8_t> > > serialized_postings;
    const std::unordered_map<std::string, PostingsList>& index_map = index.get_index();
    for (std::unordered_map<std::string, PostingsList>::const_iterator it = index_map.begin(); it != index_map.end(); ++it) {
        const std::string& term = it->first;
        const PostingsList& postings = it->second;
        std::vector<uint8_t> data;
        postings.serialize(data);
        serialized_postings.push_back(std::make_pair(term, std::move(data)));
    }

    // Current offset after header and term index
    // Term Index size: term_count * (string_len + string + offset + postings_len)
    // This is variable. Better to write term index LAST or have it fixed size.
    // Let's write Header -> Postings -> Term Index.
    // And store the offset to Term Index in the header.

    uint64_t postings_start = out.tellp();
    std::map<std::string, std::pair<uint64_t, uint32_t>> term_offsets;

    for (size_t i = 0; i < serialized_postings.size(); ++i) {
        const std::string& term = serialized_postings[i].first;
        const std::vector<uint8_t>& data = serialized_postings[i].second;
        uint64_t offset = out.tellp();
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        term_offsets[term] = std::make_pair(offset, static_cast<uint32_t>(data.size()));
    }

    uint64_t term_index_offset = out.tellp();
    for (std::map<std::string, std::pair<uint64_t, uint32_t> >::const_iterator it = term_offsets.begin(); it != term_offsets.end(); ++it) {
        const std::string& term = it->first;
        const std::pair<uint64_t, uint32_t>& info = it->second;
        uint8_t len = static_cast<uint8_t>(term.length());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(term.data(), len);
        out.write(reinterpret_cast<const char*>(&info.first), sizeof(info.first));
        out.write(reinterpret_cast<const char*>(&info.second), sizeof(info.second));
    }

    // Rewrite header with term_index_offset (need to adjust SegmentHeader)
    out.seekp(0);
    // Let's add term_index_offset to SegmentHeader
    struct {
        SegmentHeader header;
        uint64_t term_index_offset;
    } full_header = {header, term_index_offset};
    out.write(reinterpret_cast<const char*>(&full_header), sizeof(full_header));
}

FileSegmentReader::FileSegmentReader(const std::string& path) : path_(path) {
    load_index();
}

void FileSegmentReader::load_index() {
    std::ifstream in(path_, std::ios::binary);
    if (!in) return;

    SegmentHeader header;
    uint64_t term_index_offset;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    in.read(reinterpret_cast<char*>(&term_index_offset), sizeof(term_index_offset));

    if (header.magic != 0x53455243) return;

    in.seekg(term_index_offset);
    for (uint64_t i = 0; i < header.term_count; ++i) {
        uint8_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::string term(len, ' ');
        in.read(&term[0], len);
        uint64_t offset;
        uint32_t size;
        in.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        term_index_[term] = {offset, size};
    }
}

std::vector<Posting> FileSegmentReader::lookup(const std::string& term) const {
    auto it = term_index_.find(term);
    if (it == term_index_.end()) return {};

    std::ifstream in(path_, std::ios::binary);
    in.seekg(it->second.first);
    
    std::vector<uint8_t> data(it->second.second);
    in.read(reinterpret_cast<char*>(data.data()), data.size());

    const uint8_t* ptr = data.data();
    // We need to know how many postings there are to deserialize.
    // Currently PostingsList::deserialize needs num_postings.
    // I should probably store num_postings in the term index or inside the serialized data.
    // Let's check PostingsList::serialize.
    
    return PostingsList::deserialize(ptr).get_postings();
}

#ifdef _WIN32
#include <windows.h>
#endif

MmapSegmentReader::MmapSegmentReader(const std::string& path) 
    : file_handle_(NULL), mapping_handle_(NULL), data_(NULL), size_(0) {
#ifdef _WIN32
    file_handle_ = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle_ == INVALID_HANDLE_VALUE) return;

    DWORD high_size = 0;
    DWORD low_size = GetFileSize(file_handle_, &high_size);
    size_ = (static_cast<uint64_t>(high_size) << 32) | low_size;

    mapping_handle_ = CreateFileMappingA(file_handle_, NULL, PAGE_READONLY, 0, 0, NULL);
    if (mapping_handle_ == NULL) {
        CloseHandle(file_handle_);
        return;
    }

    data_ = static_cast<const uint8_t*>(MapViewOfFile(mapping_handle_, FILE_MAP_READ, 0, 0, 0));
    if (data_ == NULL) {
        CloseHandle(mapping_handle_);
        CloseHandle(file_handle_);
        return;
    }

    load_index();
#else
    // Non-windows placeholder or error
#endif
}

MmapSegmentReader::~MmapSegmentReader() {
#ifdef _WIN32
    if (data_) UnmapViewOfFile(data_);
    if (mapping_handle_) CloseHandle(mapping_handle_);
    if (file_handle_) CloseHandle(file_handle_);
#endif
}

void MmapSegmentReader::load_index() {
    if (!data_) return;

    const uint8_t* ptr = data_;
    const SegmentHeader* header = reinterpret_cast<const SegmentHeader*>(ptr);
    if (header->magic != 0x53455243) return;

    ptr += sizeof(SegmentHeader);
    uint64_t term_index_offset = *reinterpret_cast<const uint64_t*>(ptr);
    
    ptr = data_ + term_index_offset;
    for (uint64_t i = 0; i < header->term_count; ++i) {
        uint8_t len = *ptr++;
        std::string term(reinterpret_cast<const char*>(ptr), len);
        ptr += len;
        uint64_t offset = *reinterpret_cast<const uint64_t*>(ptr);
        ptr += sizeof(uint64_t);
        uint32_t size = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += sizeof(uint32_t);
        term_index_[term] = std::make_pair(offset, size);
    }
}

std::vector<Posting> MmapSegmentReader::lookup(const std::string& term) const {
    if (!data_) return {};

    std::map<std::string, std::pair<uint64_t, uint32_t> >::const_iterator it = term_index_.find(term);
    if (it == term_index_.end()) return {};

    const uint8_t* ptr = data_ + it->second.first;
    return PostingsList::deserialize(ptr).get_postings();
}

} // namespace SearchEngine
