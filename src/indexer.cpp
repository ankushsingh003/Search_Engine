#include "indexer.hpp"
#include "segment.hpp"
#include "query_executor.hpp"
#include <memory>
#include <set>
#include <algorithm>
#include <iterator>
#include <iostream>

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
    doc_store_[internal_id] = content;

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
    index.clear();
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

void Indexer::merge_segments(const std::string& output_path) {
    if (segments.empty() && index.get_index().empty()) return;

    // 1. Collect all terms
    std::set<std::string> all_terms;
    const auto& memory_index = index.get_index();
    for (auto const& pair : memory_index) {
        all_terms.insert(pair.first);
    }
    for (size_t i = 0; i < segments.size(); ++i) {
        std::vector<std::string> seg_terms = segments[i]->get_all_terms();
        all_terms.insert(seg_terms.begin(), seg_terms.end());
    }

    // 2. Create merged index
    InvertedIndex local_merged_index;

    for (const auto& term : all_terms) {
        std::map<DocID, Posting> combined; // Use map to merge/deduplicate DocIDs
        
        // From memory
        auto mem_it = memory_index.find(term);
        if (mem_it != memory_index.end()) {
            for (const auto& p : mem_it->second.get_postings()) {
                if (deleted_internal_ids_.find(p.doc_id) == deleted_internal_ids_.end()) {
                    auto& entry = combined[p.doc_id];
                    entry.doc_id = p.doc_id;
                    entry.term_freq += p.term_freq;
                    entry.positions.insert(entry.positions.end(), p.positions.begin(), p.positions.end());
                }
            }
        }

        // From segments
        for (size_t i = 0; i < segments.size(); ++i) {
            std::vector<Posting> seg_p = segments[i]->lookup(term);
            for (const auto& p : seg_p) {
                if (deleted_internal_ids_.find(p.doc_id) == deleted_internal_ids_.end()) {
                    auto& entry = combined[p.doc_id];
                    entry.doc_id = p.doc_id;
                    entry.term_freq += p.term_freq;
                    entry.positions.insert(entry.positions.end(), p.positions.begin(), p.positions.end());
                }
            }
        }

        if (!combined.empty()) {
            PostingsList pl;
            for (auto& pair : combined) {
                std::sort(pair.second.positions.begin(), pair.second.positions.end());
                pl.add_posting(pair.second.doc_id, pair.second.term_freq, pair.second.positions);
            }
            local_merged_index.add_postings_list(term, pl);
        }
    }

    // 3. Write and replace
    SegmentWriter::write(output_path, local_merged_index);
    index.clear(); 
    segments.clear();
    segments.push_back(std::unique_ptr<SegmentReader>(new MmapSegmentReader(output_path)));
    deleted_internal_ids_.clear();
}

std::map<DocID, Posting> Indexer::get_term_postings(const std::string& term) {
    std::string stemmed = PorterStemmer::stem(term);
    std::map<DocID, Posting> postings;

    // From memory
    const std::unordered_map<std::string, PostingsList>& index_map = index.get_index();
    auto memory_it = index_map.find(stemmed);
    if (memory_it != index_map.end()) {
        const std::vector<Posting>& p_list = memory_it->second.get_postings();
        for (size_t i = 0; i < p_list.size(); ++i) {
            if (deleted_internal_ids_.find(p_list[i].doc_id) == deleted_internal_ids_.end()) {
                postings[p_list[i].doc_id] = p_list[i];
            }
        }
    }

    // From segments
    for (size_t i = 0; i < segments.size(); ++i) {
        std::vector<Posting> disk_postings = segments[i]->lookup(stemmed);
        for (size_t j = 0; j < disk_postings.size(); ++j) {
            if (deleted_internal_ids_.find(disk_postings[j].doc_id) == deleted_internal_ids_.end()) {
                // Merge positions if already present (shouldn't happen with unique internal IDs across segments, 
                // but segments are independent until merged)
                if (postings.find(disk_postings[j].doc_id) == postings.end()) {
                    postings[disk_postings[j].doc_id] = disk_postings[j];
                } else {
                    auto& existing = postings[disk_postings[j].doc_id];
                    existing.term_freq += disk_postings[j].term_freq;
                    existing.positions.insert(existing.positions.end(), disk_postings[j].positions.begin(), disk_postings[j].positions.end());
                    std::sort(existing.positions.begin(), existing.positions.end());
                }
            }
        }
    }

    return postings;
}

std::string Indexer::get_document(DocID doc_id) {
    auto it = doc_store_.find(doc_id);
    if (it != doc_store_.end()) return it->second;
    return "";
}

} // namespace SearchEngine
