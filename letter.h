// Project Identifier: 50EB44D3F029ED934858FFFCEAC3547C68251FC9
#ifndef LETTER_H
#define LETTER_H

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <getopt.h>

enum class SearchMode { UNSET, STACK, QUEUE };
enum class OutputMode { WORD, MOD };

struct Options {
    SearchMode search_mode = SearchMode::UNSET;
    OutputMode output_mode = OutputMode::WORD;
    std::string beginWord, endWord;
    bool allowChange = false;
    bool allowLength = false;
    bool allowSwap = false;
};

struct WordInfo {
    std::string word;
    size_t parent = SIZE_MAX;
    bool discovered = false;
};

// parsing command line options
Options parseOptions(int argc, char* argv[]);

// dictionary helpers
std::vector<std::string> expand_complex_line(const std::string &line);

// similarity checks
bool oneLetterChange(const std::string &a, const std::string &b);
bool insertOrDelete(const std::string &a, const std::string &b);
bool swapAdjacent(const std::string &a, const std::string &b);

// modification output
std::string modificationFor(const std::string &from, const std::string &to);

// utility
std::string trim(const std::string &s);

#endif // LETTER_H