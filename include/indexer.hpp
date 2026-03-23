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
    void delete_document(DocID doc_id);
    void update_document(DocID doc_id, const std::string& content);
    void save(const std::string& path);
    void merge_segments(const std::string& output_path);
    size_t get_segment_count() const { return segments.size(); }
    std::vector<RankedResult> search(const std::string& query);
    
    std::vector<DocID> search_term(const std::string& term);
    std::set<DocID> get_all_doc_ids() const;
    size_t get_total_docs() const;
    std::map<DocID, uint32_t> get_term_tfs(const std::string& term);
    std::map<DocID, Posting> get_term_postings(const std::string& term);
    std::string get_document(DocID doc_id);

private:
    InvertedIndex& index;
    StopWordFilter stop_words;
    std::vector<std::unique_ptr<SegmentReader> > segments;
    std::set<DocID> all_internal_ids_;
    std::set<DocID> deleted_internal_ids_;
    std::map<uint32_t, DocID> external_to_internal_;
    std::map<DocID, uint32_t> internal_to_external_;
    std::map<DocID, std::string> doc_store_;
    DocID next_internal_id_;
};

} // namespace SearchEngine
