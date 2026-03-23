cmake_minimum_required(VERSION 3.10)
project(SearchEngine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(SearchEngine main.cpp ${SOURCES})

add_executable(test_pipeline tests/test_tokenizer.cpp src/tokenizer.cpp src/stop_word_filter.cpp src/porter_stemmer.cpp)
target_include_directories(test_pipeline PRIVATE include)

add_executable(test_indexing tests/test_indexing.cpp src/indexer.cpp src/inverted_index.cpp src/postings_list.cpp src/tokenizer.cpp src/stop_word_filter.cpp src/porter_stemmer.cpp src/segment.cpp src/vbyte.cpp)
target_include_directories(test_indexing PRIVATE include)

add_executable(test_vbyte tests/test_vbyte.cpp src/vbyte.cpp)
target_include_directories(test_vbyte PRIVATE include)

add_executable(test_serialization tests/test_serialization.cpp src/postings_list.cpp src/vbyte.cpp)
target_include_directories(test_serialization PRIVATE include)
