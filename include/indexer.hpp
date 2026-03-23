#pragma once

#include <string>
#include <string_view>
#include "inverted_index.hpp"
#include "tokenizer.hpp"
#include "stop_word_filter.hpp"
#include "porter_stemmer.hpp"

namespace SearchEngine {

class Indexer {
public:
    Indexer(InvertedIndex& idx);
    void index_document(DocID doc_id, std::string_view content);

private:
    InvertedIndex& index;
    StopWordFilter stop_words;
};

} // namespace SearchEngine
