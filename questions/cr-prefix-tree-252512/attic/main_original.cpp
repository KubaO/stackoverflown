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

struct BranchingInfo
{
    int branchingTotal = 0;
    int nodeCount = 0;
};

Node *root;

void insert(const std::string &word)
{
    Node **node = &root;
    for (auto ch : word) {
        if (!*node) {
            *node = new Node;
            nodeCount++;
        }
        if (ch >= 'a' && ch <= 'z')
            node = &((*node)->children[ch - 'a']);
        else
            break;
    }
    if (!*node) {
        *node = new Node;
        nodeCount++;
    }
}

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
    result.clear();
    result.reserve(lookupStream.size());
    for (char ch : lookupStream) {
        if (ch) {
            node = node->children[ch - 'a'];
            word.push_back(ch);
        } else {
            node = root;
            result.append(word);
            result.push_back('\0');
            word.clear();
        }
    }
    assert(result == lookupStream);
    return std::make_pair(std::move(result), node);
}

int main()
{
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
        insert(word);
        wordCount++;
        wordLengths.push_back(word.size());
        words.emplace_back(std::move(word));
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

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(lookupOrder.begin(), lookupOrder.end(), g);

    std::string allWords;
    allWords.reserve(4000000);
    for (auto i : lookupOrder) {
        allWords.append(words[i]);
        allWords.push_back('\0');
    }

    doLookup(root, allWords);
    auto start = std::chrono::steady_clock::now();
    doLookup(root, allWords);
    auto end = std::chrono::steady_clock::now();
    auto treeLookupTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Lookup time:" << treeLookupTime.count()    << "ms\n";

    return 0;
}
