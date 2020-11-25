#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <random>
#include <vector>
#include <fstream>
#include <iostream>

size_t nodeCount;

struct Node
{
    std::array<Node *, 1 + 'z' - 'a'> children = {};
    bool isLastChar;
    char value;
};

std::vector<Node> pool;
std::string dictionary;

Node *root;

Node *make_node()
{
    pool.emplace_back();
    return &pool.back();
}

void tree_insert(const std::string &word)
{
    Node **node = &root;
    for (auto ch : word) {
        if (!*node) {
            *node = make_node();
            nodeCount++;
        }
        if (ch >= 'a' && ch <= 'z')
            node = &((*node)->children[ch - 'a']);
        else
            break;
    }
    if (!*node) {
        *node = make_node();
        nodeCount++;
    }
}

struct BranchingInfo
{
    int branchingTotal = 0;
    int nodeCount = 0;
};


std::vector<BranchingInfo> branching(100);

void tally_branching(Node *node, int level = 0)
{
    for (auto *child : node->children) {
        if (!child)
            continue;
        branching[level].branchingTotal++;
        tally_branching(child, level + 1);
    }
    branching[level].nodeCount++;
}

auto doLookup(Node *root, const std::string &lookupStream)
{
    std::string result, word;
    Node *node = root;
    result.reserve(lookupStream.size());
    for (char ch : lookupStream) {
        if (ch) {
            node = node->children[ch - 'a'];
            word.push_back(ch);
        } else {
            node = root; // found the prefix
            result.append(word);
            result.push_back('\0');
            word.clear();
        }
    }
    assert(result == lookupStream);
    return std::make_pair(std::move(result), node);
}

auto doLookup2(const std::string &dictionary, const std::string &lookupStream)
{
    assert(dictionary.front() == '\0');
    assert(dictionary.back() == '\0');
    assert(lookupStream.back() == '\0');
    std::string result, word;
    result.reserve(lookupStream.size() * 10);
    for (char ch : lookupStream) {
        if (ch)
            word.push_back(ch);
        else {
            const char *begin = &dictionary.front(), *end = &dictionary.back();
            const char *prevP = nullptr;
            for (;;) { // binary search
                const char *p = begin + (end - begin) / 2;
                const char *const p0 = p;
                assert(p0 != prevP); // otherwise we didn't find the word
                while (*p)
                    --p; // find start of the word
                p++;
                int const cmp = strcmp(word.data(), p);
                if (cmp == 0) {
                    // found the word, but not necessarily the beginning of the prefix
                    assert(word == p);
                    //                    result.append(word);
                    //                    result.push_back('\0');
                    // find the first word that would be the prefix
                    const char *prefixWord = p;
                    while (p > (&dictionary.front() + 1)) {
                        p--;
                        while (*p)
                            p--;
                        const char *maybePrefix = p;
                        char diff = 0;
                        for (char w : word)
                            diff |= (*maybePrefix++ ^ w);
                        if (diff) // prefix doesn't match
                            break;
                        prefixWord = p;
                    }
                    result.append(prefixWord);
                    result.push_back('\0');
                    word.clear();
                    break;
                }
                if (cmp < 0 /* word < p */) {
                    end = p0;
                } else { /* word > p */
                    begin = p0;
                }
                prevP = p0;
            }
        }
    }
    //assert(result == lookupStream);
    return result;
}

using Letter = unsigned int;
constexpr int l_max = sizeof(Letter) * 8;

auto make_dictionary(const std::vector<std::string> &words)
{
    assert(std::is_sorted(words.begin(), words.end()));
    std::vector<Letter> dict;
    dict.push_back(0);
    for (auto &word : words) {
        Letter l;
        int n = 0;
        for (char w : word) {
            l |= Letter(w) << n;
            n += 8;
            if (n == l_max) {
                dict.push_back(l);
                l = 0;
                n = 0;
            }
        }
        if (l)
            dict.push_back(l);
        dict.push_back(0);
    }
    return dict;
}

auto doLookup3(const std::vector<Letter> &dictionary, const std::string &lookupStream)
{
    assert(dictionary.front() == '\0');
    assert(dictionary.back() == '\0');
    assert(lookupStream.back() == '\0');
    std::string result, word;
    result.reserve(lookupStream.size() * 10);
    for (char ch : lookupStream) {
        if (ch)
            word.push_back(ch);
        else {
            const Letter *begin = &dictionary.front(), *end = &dictionary.back();
            const Letter *prevP = nullptr;
            for (;;) { // binary search
                const Letter *p = begin + (end - begin) / 2;
                const Letter *const p0 = p;
                assert(p0 != prevP); // otherwise we didn't find the word
                while (*p)
                    --p; // find start of the word
                p++;
                int const cmp = strcmp(word.data(), (const char *)p);
                if (cmp == 0) {
                    // found the word, but not necessarily the beginning of the prefix
                    assert(word == (const char *)p);
                    //                    result.append(word);
                    //                    result.push_back('\0');
                    // find the first word that would be the prefix
                    const Letter *prefixWord = p;
                    while (p > (&dictionary.front() + 1)) {
                        p--;
                        while (*p)
                            p--;
                        auto *maybePrefix = (const char *)p;
                        char diff = 0;
                        for (char w : word)
                            diff |= (*maybePrefix++ ^ w);
                        if (diff) // prefix doesn't match
                            break;
                        prefixWord = p;
                    }
                    result.append((const char *)prefixWord);
                    result.push_back('\0');
                    word.clear();
                    break;
                }
                if (cmp < 0 /* word < p */) {
                    end = p0;
                } else { /* word > p */
                    begin = p0;
                }
                prevP = p0;
            }
        }
    }
    //assert(result == lookupStream);
    return result;
}


int main()
{
    pool.reserve(1100000);

    std::vector<std::string> words;
    std::vector<char> wordLengths;
    size_t letterCount = 0;
    size_t wordCount = 0;
    std::ifstream in("C:\\wc\\words_alpha.txt");

    words.reserve(371000);
    wordLengths.reserve(371000);

    std::string word;
    while (in >> word) {
        letterCount += word.size();
        tree_insert(word);
        wordCount++;
        wordLengths.push_back(word.size());
        words.emplace_back(std::move(word));
    }

    std::sort(words.begin(), words.end());
    assert(std::is_sorted(words.begin(), words.end()));
    dictionary.push_back('\0');
    for (auto const &word : words) {
        dictionary.append(word);
        dictionary.push_back('\0');
    }

    std::sort(wordLengths.begin(), wordLengths.end());
    int medianWordLength = wordLengths[wordLengths.size() / 2];
    int maxWordLength = wordLengths.back();

    tally_branching(root);

    std::cout << sizeof(Node) << "\n";
    std::cout << letterCount << " " << wordCount << " " << nodeCount << "\n";
    std::cout << medianWordLength << " " << maxWordLength << " " << size_t(nodeCount) * sizeof(Node)
              << "\n";

    int level = 0;
    double branchProduct = 1;
    for (auto const &bi : branching) {
        if (bi.nodeCount == 0)
            break;
        auto branchFactor = double(bi.branchingTotal) / bi.nodeCount;
        branchProduct *= branchFactor;
        std::cout << level++ << ":" << branchFactor << "," << bi.nodeCount << "," << branchProduct
                  << "\n";
    }

    std::vector<int> lookupOrder;
    for (size_t i = 0; i < wordCount; ++i)
        lookupOrder.push_back(i);

    //std::random_device rd;
    std::mt19937 g(1);

    std::shuffle(lookupOrder.begin(), lookupOrder.end(), g);

    std::string allWords;
    allWords.reserve(4000000);
    for (auto i : lookupOrder) {
        allWords.append(words[i]);
        allWords.push_back('\0');
    }

    auto dict = make_dictionary(words);
    doLookup3(dict, allWords);
    auto start = std::chrono::steady_clock::now();
    doLookup3(dict, allWords);
    auto end = std::chrono::steady_clock::now();
    auto treeLookupTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Lookup time:" << treeLookupTime.count()    << "ms\n";

    return 0;
}
