#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <unordered_map>
#include "inverted_index.hpp"
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include "porter_stemmer.hpp"
#include "postings_list.hpp"
#include "segment.hpp"
#include "ranker.hpp"

namespace SearchEngine {

class Indexer {
public:
    Indexer(InvertedIndex& idx);
    void index_document(DocID doc_id, const std::string& content);
    void save(const std::string& path);
    std::vector<RankedResult> search(const std::string& query);
    
    std::vector<DocID> search_term(const std::string& term);
    std::set<DocID> get_all_doc_ids() const;
    size_t get_total_docs() const;
    std::map<DocID, uint32_t> get_term_tfs(const std::string& term);

private:
    InvertedIndex& index;
    StopWordFilter stop_words;
    std::vector<std::unique_ptr<SegmentReader> > segments;
    std::set<DocID> all_doc_ids_;
};

} // namespace SearchEngine
