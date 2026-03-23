#pragma once

#include <string>
#include <vector>
#include <memory>
#include "inverted_index.hpp"
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include "porter_stemmer.hpp"
#include "postings_list.hpp"
#include "segment.hpp"

namespace SearchEngine {

class Indexer {
public:
    Indexer(InvertedIndex& idx);
    void index_document(DocID doc_id, const std::string& content);
    void save(const std::string& path);
    std::vector<DocID> search(const std::string& query);

private:
    InvertedIndex& index;
    StopWordFilter stop_words;
    std::vector<std::unique_ptr<SegmentReader> > segments;
};

} // namespace SearchEngine
