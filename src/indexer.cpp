#include "indexer.hpp"
#include "segment.hpp"
#include "query_executor.hpp"
#include <memory>
#include <set>
#include <algorithm>
#include <iterator>

namespace SearchEngine {

Indexer::Indexer(InvertedIndex& idx) : index(idx), next_internal_id_(0) {}

void Indexer::delete_document(DocID external_id) {
    auto it = external_to_internal_.find(external_id);
    if (it != external_to_internal_.end()) {
        deleted_internal_ids_.insert(it->second);
        external_to_internal_.erase(it);
    }
}

void Indexer::update_document(DocID external_id, const std::string& content) {
    // delete_document handles external_to_internal_ cleanup and deleted_internal_ids_ insertion
    delete_document(external_id);
    index_document(external_id, content);
}

void Indexer::index_document(DocID external_id, const std::string& content) {
    // If external_id already exists, mark its current internal version as deleted
    auto it = external_to_internal_.find(external_id);
    if (it != external_to_internal_.end()) {
        deleted_internal_ids_.insert(it->second);
    }

    DocID internal_id = next_internal_id_++;
    external_to_internal_[external_id] = internal_id;
    internal_to_external_[internal_id] = external_id;
    all_internal_ids_.insert(internal_id);

    auto tokens = Tokenizer::tokenize(content);
    stop_words.filter(tokens);

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string stemmed = PorterStemmer::stem(tokens[i]);
        index.add_term(stemmed, internal_id, static_cast<Position>(i));
    }
}

void Indexer::save(const std::string& path) {
    SegmentWriter::write(path, index);
    segments.push_back(std::unique_ptr<SegmentReader>(new MmapSegmentReader(path)));
}

std::vector<RankedResult> Indexer::search(const std::string& query) {
    std::vector<DocID> matches = QueryExecutor::execute(query, *this);
    std::vector<RankedResult> internal_results = Ranker::rank(matches, query, *this);
    
    // Map internal IDs back to external IDs
    std::vector<RankedResult> external_results;
    for (size_t i = 0; i < internal_results.size(); ++i) {
        auto it = internal_to_external_.find(internal_results[i].doc_id);
        if (it != internal_to_external_.end()) {
            RankedResult res;
            res.doc_id = it->second;
            res.score = internal_results[i].score;
            external_results.push_back(res);
        }
    }
    return external_results;
}

std::vector<DocID> Indexer::search_term(const std::string& term) {
    std::string stemmed = PorterStemmer::stem(term);
    std::set<DocID> term_internal_docs;

    // Check in-memory index
    const std::unordered_map<std::string, PostingsList>& index_map = index.get_index();
    auto memory_it = index_map.find(stemmed);
    if (memory_it != index_map.end()) {
        const std::vector<Posting>& p_list = memory_it->second.get_postings();
        for (size_t i = 0; i < p_list.size(); ++i) {
            if (deleted_internal_ids_.find(p_list[i].doc_id) == deleted_internal_ids_.end()) {
                term_internal_docs.insert(p_list[i].doc_id);
            }
        }
    }

    // Check disk segments
    for (size_t i = 0; i < segments.size(); ++i) {
        std::vector<Posting> disk_postings = segments[i]->lookup(stemmed);
        for (size_t j = 0; j < disk_postings.size(); ++j) {
            if (deleted_internal_ids_.find(disk_postings[j].doc_id) == deleted_internal_ids_.end()) {
                term_internal_docs.insert(disk_postings[j].doc_id);
            }
        }
    }

    return std::vector<DocID>(term_internal_docs.begin(), term_internal_docs.end());
}

std::set<DocID> Indexer::get_all_doc_ids() const {
    // Returns active internal IDs
    std::set<DocID> active_docs;
    for (auto it = all_internal_ids_.begin(); it != all_internal_ids_.end(); ++it) {
        if (deleted_internal_ids_.find(*it) == deleted_internal_ids_.end()) {
            active_docs.insert(*it);
        }
    }
    return active_docs;
}

size_t Indexer::get_total_docs() const {
    return get_all_doc_ids().size();
}

std::map<DocID, uint32_t> Indexer::get_term_tfs(const std::string& term) {
    std::string stemmed = PorterStemmer::stem(term);
    std::map<DocID, uint32_t> tfs;

    // From memory
    const std::unordered_map<std::string, PostingsList>& index_map = index.get_index();
    auto memory_it = index_map.find(stemmed);
    if (memory_it != index_map.end()) {
        const std::vector<Posting>& p_list = memory_it->second.get_postings();
        for (size_t i = 0; i < p_list.size(); ++i) {
            if (deleted_internal_ids_.find(p_list[i].doc_id) == deleted_internal_ids_.end()) {
                tfs[p_list[i].doc_id] += p_list[i].term_freq;
            }
        }
    }

    // From segments
    for (size_t i = 0; i < segments.size(); ++i) {
        std::vector<Posting> disk_postings = segments[i]->lookup(stemmed);
        for (size_t j = 0; j < disk_postings.size(); ++j) {
            if (deleted_internal_ids_.find(disk_postings[j].doc_id) == deleted_internal_ids_.end()) {
                tfs[disk_postings[j].doc_id] += disk_postings[j].term_freq;
            }
        }
    }

    return tfs;
}

} // namespace SearchEngine
