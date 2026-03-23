# Mini_Search: High-Performance C++ Search Engine

A production-pattern search engine with inverted indexing, TF-IDF ranking, disk persistence (mmap), and Boolean/phrase query support.

## Project Structure
- `include/`: Header files (.hpp)
- `src/`: Source files (.cpp)
- `tests/`: Unit and integration tests
- `main.cpp`: Entry point

## Roadmap
1. **Week 1-2**: Tokenizer, PorterStemmer, StopWordFilter.
2. **Week 3-4**: InvertedIndex (hash map from term to PostingsList).
3. **Week 5**: VByteCodec (integer compression).
4. **Week 6-7**: Persistence (disk segments, mmap).
5. **Week 8-9**: QueryParser (recursive descent) and QueryExecutor.
6. **Week 10**: TfIdfRanker.
7. **Week 11-12**: DeltaUpdates (segment merging).
