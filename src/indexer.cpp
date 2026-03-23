#include "indexer.hpp"

namespace SearchEngine {

Indexer::Indexer(InvertedIndex& idx) : index(idx) {}

void Indexer::index_document(DocID doc_id, std::string_view content) {
    auto tokens = Tokenizer::tokenize(content);
    stop_words.filter(tokens);

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string stemmed = PorterStemmer::stem(tokens[i]);
        index.add_term(stemmed, doc_id, static_cast<Position>(i));
    }
}

} // namespace SearchEngine
