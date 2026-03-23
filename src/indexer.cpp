#include "indexer.hpp"
#include "segment.hpp"
#include "query_executor.hpp"
#include <memory>
#include <set>
#include <algorithm>
#include <iterator>

namespace SearchEngine {

Indexer::Indexer(InvertedIndex& idx) : index(idx) {}

void Indexer::index_document(DocID doc_id, const std::string& content) {
    all_doc_ids_.insert(doc_id);
    auto tokens = Tokenizer::tokenize(content);
    stop_words.filter(tokens);

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string stemmed = PorterStemmer::stem(tokens[i]);
        index.add_term(stemmed, doc_id, static_cast<Position>(i));
    }
}

void Indexer::save(const std::string& path) {
    SegmentWriter::write(path, index);
    segments.push_back(std::unique_ptr<SegmentReader>(new MmapSegmentReader(path)));
}

std::vector<DocID> Indexer::search(const std::string& query) {
    return QueryExecutor::execute(query, *this);
}

std::vector<DocID> Indexer::search_term(const std::string& term) {
    std::string stemmed = PorterStemmer::stem(term);
    std::set<DocID> term_docs;

    // Check in-memory index
    const std::unordered_map<std::string, PostingsList>& index_map = index.get_index();
    std::unordered_map<std::string, PostingsList>::const_iterator memory_it = index_map.find(stemmed);
    if (memory_it != index_map.end()) {
        const std::vector<Posting>& p_list = memory_it->second.get_postings();
        for (size_t i = 0; i < p_list.size(); ++i) {
            term_docs.insert(p_list[i].doc_id);
        }
    }

    // Check disk segments
    for (size_t i = 0; i < segments.size(); ++i) {
        std::vector<Posting> disk_postings = segments[i]->lookup(stemmed);
        for (size_t j = 0; j < disk_postings.size(); ++j) {
            term_docs.insert(disk_postings[j].doc_id);
        }
    }

    return std::vector<DocID>(term_docs.begin(), term_docs.end());
}

std::set<DocID> Indexer::get_all_doc_ids() const {
    return all_doc_ids_;
}

} // namespace SearchEngine
