// Project Identifier: 50EB44D3F029ED934858FFFCEAC3547C68251FC9

#include "letter.h"

using namespace std;

static void usage_and_exit(int code = 0) {
    if (code == 0) {
        cout << "Usage: letter [options] < dictionary\n";
        cout << " --stack/-s or --queue/-q (exactly one)\n";
        cout << " --begin/-b <word> and --end/-e <word> (both required)\n";
        cout << " at least one of --change/-c, --length/-l, or --swap/-p\n";
        cout << " --output/-o (W|M) optional, default W\n";
        exit(0);
    }
    else {
        cerr << "Try 'letter --help' for more information.\n";
        exit(1);
    }
}

Options parseOptions(int argc, char* argv[]) {
    Options opts;
    const option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"stack", no_argument, nullptr, 's'},
        {"queue", no_argument, nullptr, 'q'},
        {"begin", required_argument, nullptr, 'b'},
        {"end", required_argument, nullptr, 'e'},
        {"change", no_argument, nullptr, 'c'},
        {"length", no_argument, nullptr, 'l'},
        {"swap", no_argument, nullptr, 'p'},
        {"output", required_argument, nullptr, 'o'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hsqb:e:clpo:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                usage_and_exit(0);
                break;
            case 's':
                if (opts.search_mode != SearchMode::UNSET) {
                    cerr << "Conflicting or duplicate stack and queue specified\n";
                    exit(1);
                }
                opts.search_mode = SearchMode::STACK;
                break;
            case 'q':
                if (opts.search_mode != SearchMode::UNSET) {
                    cerr << "Conflicting or duplicate stack and queue specified\n";
                    exit(1);
                }
                opts.search_mode = SearchMode::QUEUE;
                break;
            case 'b':
                opts.beginWord = optarg;
                break;
            case 'e':
                opts.endWord = optarg;
                break;
            case 'c':
                opts.allowChange = true;
                break;
            case 'l':
                opts.allowLength = true;
                break;
            case 'p':
                opts.allowSwap = true;
                break;
            case 'o':
                if (string(optarg) == "W") {
                    opts.output_mode = OutputMode::WORD;
                } else if (string(optarg) == "M") {
                    opts.output_mode = OutputMode::MOD;
                } else {
                    cerr << "Invalid output mode specified\n";
                    exit(1);
                }
                break;
            default:
                cerr << "Unknown command line option\n";
                exit(1);
        }
    }

    // Validate options
    if (opts.search_mode == SearchMode::UNSET) {
        cerr << "Must specify one of stack or queue\n";
        exit(1);
    }
    if (opts.beginWord.empty()) {
        cerr << "Beginning word not specified\n";
        exit(1);
    }
    if (opts.endWord.empty()) {
        cerr << "Ending word not specified\n";
        exit(1);
    }
    if (!opts.allowChange && !opts.allowLength && !opts.allowSwap) {
        cerr << "Must specify at least one modification mode (change length swap)\n";
        exit(1);
    }
    if (!opts.allowLength && (opts.beginWord.size() != opts.endWord.size())) {
        cerr << "The first and last words must have the same length when length mode is off\n";
        exit(1);
    }

    return opts;
}

string trim (const string &s) {
    size_t a = 0;
    size_t b = s.size();

    while (a < b && isspace(static_cast<unsigned char>(s[a]))) {
        ++a;
    }
    while (b > a && isspace(static_cast<unsigned char>(s[b - 1]))) {
        --b;
    }

    return s.substr(a, b - a);
}

vector<string> expand_complex_line(const string &line) {
    if(line.find_first_of("&[]!?") == string::npos) {
        return {line};
    }

    vector<string> final_results;
    size_t amp = line.find('&');
    size_t l_bracket = line.find('[');
    size_t excl = line.find('!');
    size_t q_mark = line.find('?');

    if (amp != string::npos && (amp < l_bracket || l_bracket == string::npos) &&
        (amp < excl || excl == string::npos) && (amp < q_mark || q_mark == string::npos)) {
        string base = line.substr(0, line.size() - 1);
        string rev = base;
        reverse(rev.begin(), rev.end());

        auto expanded_base = expand_complex_line(base);
        final_results.insert(final_results.end(), expanded_base.begin(), expanded_base.end());

        auto expanded_rev = expand_complex_line(rev);
        final_results.insert(final_results.end(), expanded_rev.begin(), expanded_rev.end());
    }
    else if (l_bracket != string::npos && (l_bracket < excl || excl == string::npos) &&
             (l_bracket < q_mark || q_mark == string::npos)) {
        size_t r_bracket = line.find(']', l_bracket);
        string before = line.substr(0, l_bracket);
        string choices = line.substr(l_bracket + 1, r_bracket - l_bracket - 1);
        string after = line.substr(r_bracket + 1);

        for (char c : choices) {
            auto recursive_results = expand_complex_line(before + c + after);
            final_results.insert(final_results.end(), recursive_results.begin(), recursive_results.end());
        }
    }
    else if (excl != string::npos && (excl < q_mark || q_mark == string::npos)) {
        string prefix = line.substr(0, excl);
        string suffix = line.substr(excl + 1);
        if (prefix.length() >= 2) {
            string swapped = prefix;
            swap(swapped[swapped.length() - 1], swapped[swapped.length() - 2]);
            
            auto expanded_swapped = expand_complex_line(swapped + suffix);
            final_results.insert(final_results.end(), expanded_swapped.begin(), expanded_swapped.end());

            auto expanded_prefix = expand_complex_line(prefix + suffix);
            final_results.insert(final_results.end(), expanded_prefix.begin(), expanded_prefix.end());
        }
        else {
            auto expanded_prefix = expand_complex_line(prefix + suffix);
            final_results.insert(final_results.end(), expanded_prefix.begin(), expanded_prefix.end());
        }
    }
    else if (q_mark != string::npos) {
        string prefix = line.substr(0, q_mark);
        string suffix = line.substr(q_mark + 1);
        if (!prefix.empty()) {
            string doubled = prefix + prefix.back();

            auto expanded_normal = expand_complex_line(prefix + suffix);
            final_results.insert(final_results.end(), expanded_normal.begin(), expanded_normal.end());

            auto expanded_doubled = expand_complex_line(doubled + suffix);
            final_results.insert(final_results.end(), expanded_doubled.begin(), expanded_doubled.end());
        }
        else {
            auto expanded_normal = expand_complex_line(prefix + suffix);
            final_results.insert(final_results.end(), expanded_normal.begin(), expanded_normal.end());
        }
    }

    return final_results;
}

bool oneLetterChange(const string &a, const string &b) {
    if (a.size() != b.size()) {
        return false;
    }
    int diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            ++diff;
            if (diff > 1) {
                return false;
            }
        }
    }

    return diff == 1;
}

bool insertOrDelete(const string &a, const string &b) {
    if (a.size() == b.size()) {
        return false;
    }
    if ((a.size() + 1 != b.size()) && (b.size() + 1 != a.size())) {
        return false;
    }

    const string &s = (a.size() < b.size()) ? a : b;
    const string &l = (a.size() < b.size()) ? b : a;

    size_t i = 0, j = 0;
    bool skipped = false;
    while (i < s.size() && j < l.size()) {
        if (s[i] == l[j]) {
            ++i;
            ++j;
        }
        else if (!skipped) {
            skipped = true;
            ++j;
        }
        else {
            return false;
        }
    }
    return true;
}

bool swapAdjacent(const string &a, const string &b) {
    if (a.size() != b.size()) {
        return false;
    }
    
    if (a.size() < 2) {
        return false;
    }

    for (size_t i = 0; i + 1 < a.size(); ++i) {
        string temp = a;
        swap(temp[i], temp[i + 1]);
        if (temp == b && a != b) {
            return true;
        }
    }

    return false;
}

string modificationFor(const string &from, const string &to) {
    if (from.size() == to.size()) {
        for (size_t i = 0; i + 1 < from.size(); ++i) {
            string temp = from;
            swap(temp[i], temp[i + 1]);
            if (temp == to) {
                return "s," + to_string(i);
            }
        }

        size_t pos = 0;
        while (pos < from.size() && from[pos] == to[pos]) {
            ++pos;
        }
        
        if (pos < from.size()) {
            return "c," + to_string(pos) + "," + string(1, to[pos]);
        }

        return "";
    }

    if (from.size() + 1 == to.size()) {
        size_t i = 0, j = 0;
        while (i < from.size() && from[i] == to[j]) {
            ++i;
            ++j;
        }

        if (j < to.size()) {
            return "i," + to_string(j) + "," + string(1, to[j]);
        }
        else {
            return "i," + to_string(j) + "," + string(1, to.back());
        }
    }

    if (to.size() + 1 == from.size()) {
        size_t i = 0, j = 0;
        while (j < to.size() && from[i] == to[j]) {
            ++i;
            ++j;
        }

        return "d," + to_string(i);
    }

    return "";
}