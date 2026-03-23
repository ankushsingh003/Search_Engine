#include <iostream>
#include <cassert>
#include <set>
#include <string>
#include "snippet_generator.hpp"

using namespace SearchEngine;

void test_simple_snippet() {
    std::string doc = "The search engine should be able to find relevant documents and show a snippet of text with the matching terms highlighted.";
    std::set<std::string> queries = {"engine", "highlighted"};
    
    std::string snippet = SnippetGenerator::generate(doc, queries, 10);
    std::cout << "Snippet: " << snippet << std::endl;
    
    // Check if matching terms are highlighted
    assert(snippet.find("**engine**") != std::string::npos);
    // Depending on the window, 'highlighted' might or might not be there if window is small
}

void test_no_matches() {
    std::string doc = "This is a simple test document.";
    std::set<std::string> queries = {"nonexistent"};
    
    std::string snippet = SnippetGenerator::generate(doc, queries, 10);
    std::cout << "No match snippet: " << snippet << std::endl;
    assert(snippet.find("**") == std::string::npos);
}

int main() {
    test_simple_snippet();
    test_no_matches();
    std::cout << "Snippet generator tests passed!" << std::endl;
    return 0;
}
