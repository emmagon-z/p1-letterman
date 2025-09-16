// Project Identifier: 50EB44D3F029ED934858FFFCEAC3547C68251FC9

#include "letter.h"
#include <unordered_map>
using namespace std;

int main (int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Options opts = parseOptions(argc, argv);

    string line;
    if (!getline(cin, line)) {
        cerr << "Empty input\n";
        exit(1);
    }
    line = trim(line);
    char dictType = line.empty() ? 'S' : line[0];
    if (!getline(cin, line)) {
        cerr << "Missing size line\n";
        exit(1);
    }
    line = trim(line);
    long long declared_n = 0;
    try {
        declared_n = stoll(line);
    }
    catch (...) {
        cerr << "Invalid size line\n";
        exit(1);
    }

    vector<WordInfo> dictionary;
    if (dictType == 'C') {
        dictionary.reserve(static_cast<size_t>(max(0LL, declared_n) * 3));
    }
    else {
        dictionary.reserve(static_cast<size_t>(max(0LL, declared_n)));
    }

    long long read_count = 0;
    while(read_count < declared_n && getline(cin, line)) {
        if (line.empty()) {
            break;
        }
        
        string t = trim(line);
        if (t.empty()) {
            break;
        }
        if (t.rfind("//", 0) == 0) {
            continue;
        }
        if (dictType == 'S') {
            if (!opts.allowLength && t.length() != opts.beginWord.length()) {
                continue;
            }
            dictionary.push_back({t});
        }
        else {
            auto exp = expand_complex_line(t);
            for (auto &w : exp) {
                if (!opts.allowLength && w.length() != opts.beginWord.length()) {
                    continue;
                }
                dictionary.push_back({w});
            }
        }
        ++read_count;
    }

    size_t beginIndex = SIZE_MAX, endIndex = SIZE_MAX;
    for (size_t i = 0; i < dictionary.size(); ++i) {
        if (dictionary[i].word == opts.beginWord) {
            beginIndex = i;
        }
        if (dictionary[i].word == opts.endWord) {
            endIndex = i;
        }
    }
    if (beginIndex == SIZE_MAX) {
        cerr << "Beginning word does not exist in the dictionary\n";
        exit(1);
    }
    if (endIndex == SIZE_MAX) {
        cerr << "Ending word does not exist in the dictionary\n";
        exit(1);
    }

    unordered_map<size_t, vector<size_t>> buckets;
    for (size_t i = 0; i < dictionary.size(); ++i) {
        buckets[dictionary[i].word.size()].push_back(i);
    }

    deque<size_t> sc;
    dictionary[beginIndex].discovered = true;
    dictionary[beginIndex].parent = SIZE_MAX;
    sc.push_back(beginIndex);

    size_t discovered_count = 1;
    bool solution_found = false;

    while (!sc.empty()) {
        size_t current;
        if (opts.search_mode == SearchMode::STACK) {
            current = sc.back();
            sc.pop_back();
        }
        else {
            current = sc.front();
            sc.pop_front();
        }

        if (current == endIndex) {
            solution_found = true;
            break;
        }

        const string &curWord = dictionary[current].word;
        size_t curLen = curWord.size();
        
        vector<size_t> candidateLengths;
        candidateLengths.reserve(3);
        candidateLengths.push_back(curLen);
        if (opts.allowLength) {
            candidateLengths.push_back(curLen + 1);
            if (curLen > 0) {
                candidateLengths.push_back(curLen - 1);
            }
        }

        vector<size_t> neighborsToAdd;

        for (size_t len : candidateLengths) {
            auto it = buckets.find(len);
            if (it == buckets.end()) {
                continue;
            }
            
            for (size_t i : it->second) {
                if (dictionary[i].discovered) {
                    continue;
                }
                const string &candidate = dictionary[i].word;
                
                bool similar = false;
                if (opts.allowSwap && swapAdjacent(curWord, candidate)) {
                    similar = true;
                }
                if (!similar && opts.allowChange && oneLetterChange(curWord, candidate)) {
                    similar = true;
                }
                if (!similar && opts.allowLength && insertOrDelete(curWord, candidate)) {
                    similar = true;
                }

                if (similar) {
                    dictionary[i].discovered = true;
                    dictionary[i].parent = current;
                    ++discovered_count;

                    if (opts.search_mode == SearchMode::QUEUE) {
                        sc.push_back(i);
                    }
                    else {
                        neighborsToAdd.push_back(i);
                    }
                }
            }
        }
        if (opts.search_mode == SearchMode::STACK) {
            for (auto it = neighborsToAdd.rbegin(); it != neighborsToAdd.rend(); ++it) {
                sc.push_back(*it);
            }
        }
    }

    if (!solution_found) {
        cout << "No solution, " << discovered_count << " words discovered.\n";
        return 0;
    }

    vector<size_t> path;
    for (size_t current = endIndex; current != SIZE_MAX; current = dictionary[current].parent) {
        path.push_back(current);
    }
    reverse(path.begin(), path.end());

    cout << "Words in morph: " << path.size() << '\n';
    if (opts.output_mode == OutputMode::WORD) {
        for (size_t idx : path) {
            cout << dictionary[idx].word << '\n';
        }
    }
    else {
        cout << dictionary[path[0]].word << '\n';
        for (size_t i = 1; i < path.size(); ++i) {
            cout << modificationFor(dictionary[path[i - 1]].word, dictionary[path[i]].word) << '\n';
        }
    }

    return 0;
}