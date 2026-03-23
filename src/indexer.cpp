#include "indexer.hpp"
#include "segment.hpp"
#include <memory>
#include <set>
#include <algorithm>
#include <iterator>

namespace SearchEngine {

Indexer::Indexer(InvertedIndex& idx) : index(idx) {}

void Indexer::index_document(DocID doc_id, const std::string& content) {
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
    // Optionally clear in-memory index after saving if we want to save memory
}

std::vector<DocID> Indexer::search(const std::string& query) {
    auto tokens = Tokenizer::tokenize(query);
    stop_words.filter(tokens);
    if (tokens.empty()) return {};

    std::set<DocID> results;
    bool first = true;

    for (const auto& token : tokens) {
        std::string stemmed = PorterStemmer::stem(token);
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

        if (first) {
            results = std::move(term_docs);
            first = false;
        } else {
            std::set<DocID> intersection;
            std::set_intersection(results.begin(), results.end(),
                                  term_docs.begin(), term_docs.end(),
                                  std::inserter(intersection, intersection.begin()));
            results = std::move(intersection);
        }
    }

    return std::vector<DocID>(results.begin(), results.end());
}

} // namespace SearchEngine
