#define _CRT_SECURE_NO_WARNINGS 1
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stopwatchReset();
size_t stopwatchElapsed();
char *permuteWordList(char *words, size_t wordCount);

#define NODE_ENDWORD_FLAG 0x80000000U
#define NODE_LOWEST_VALUE 'a'
#define NODE_HIGHEST_VALUE 'z'
#define NODE_VALUE_SPAN (1 + NODE_HIGHEST_VALUE - NODE_LOWEST_VALUE)

struct
{
    size_t allocNodes;
    size_t nodes;
    size_t allocMem;
    size_t reallocCount;
} nodeStatus;

inline int numberOfSetBits(uint32_t i)
{
    // source: https://stackoverflow.com/a/109025/1329652
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

inline int numberOfSetBitsBelow(uint32_t bits, uint32_t i)
{
    return numberOfSetBits(bits & ((1 << i) - 1));
}

struct TreeNode
{
    uint32_t activeChildren;
    struct TreeNode *children[1];
} typedef TreeNode;

TreeNode stockNode;
TreeNode stockLeafNode;

inline bool nodeEndsWord(const TreeNode *node)
{
    return node->activeChildren & NODE_ENDWORD_FLAG;
}

inline bool bitIsSet(uint32_t bits, int bit)
{
    assert(bit >= 0 && bit < NODE_VALUE_SPAN);
    return bits & (1 << bit);
}

inline void setNodeEndsWord(TreeNode *node)
{
    node->activeChildren |= NODE_ENDWORD_FLAG;
}

inline void setAnyNodeEndsWord(TreeNode **nodePtr)
{
    TreeNode *const node = *nodePtr;
    if (node == &stockNode)
        *nodePtr = &stockLeafNode;
    else if (node != &stockLeafNode)
        setNodeEndsWord(node);
}

int nodeSize(const TreeNode *node)
{
    return numberOfSetBits(node->activeChildren & ~NODE_ENDWORD_FLAG);
}

int nodeCapacity(int nodeSize)
{
    static const int8_t capacity[27] = {1,  1,  2,  4,  4,  8,  8,  8,  8,  16, 16, 16, 16, 16,
                                        16, 16, 16, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26};
    assert(nodeSize >= 0 && nodeSize <= 26);
    return capacity[nodeSize];
}

int nodeBytes(int nodeCapacity)
{
    assert(nodeCapacity > 0);
    return sizeof(TreeNode) + (nodeCapacity - 1) * sizeof(TreeNode *);
}

TreeNode *createNode(void)
{
    TreeNode *node = malloc(sizeof(TreeNode));
    if (node) {
        node->activeChildren = 0;
        nodeStatus.allocMem += sizeof(TreeNode);
        nodeStatus.allocNodes++;
        nodeStatus.nodes++;
    }
    return node;
}

void destroyNode(TreeNode **nodePtr)
{
    if (!nodePtr)
        return;
    TreeNode *node = *nodePtr;
    if (node) {
        *nodePtr = NULL;
        nodeStatus.nodes--;
        if (node != &stockNode && node != &stockLeafNode) {
            int const size = nodeSize(node);
            for (int i = size - 1; i >= 0; i--) {
                destroyNode(node->children + i);
            }
            nodeStatus.allocMem -= nodeBytes(nodeCapacity(size));
            nodeStatus.allocNodes--;
            free(node);
        }
    }
}

void setupStockNodes(void)
{
    setNodeEndsWord(&stockLeafNode);
}

TreeNode *makeNodeWritable(TreeNode *node)
{
    if (node == &stockNode) {
        node = createNode();
        if (node)
            nodeStatus.nodes--;
    } else if (node == &stockLeafNode) {
        node = createNode();
        if (node) {
            nodeStatus.nodes--;
            setNodeEndsWord(node);
        }
    }
    return node;
}

TreeNode **getChildNode(TreeNode *node, char ch)
{
    ch -= NODE_LOWEST_VALUE;
    uint32_t childrenBits = node->activeChildren;
    if (!bitIsSet(childrenBits, ch))
        return NULL;
    return node->children + numberOfSetBitsBelow(childrenBits, ch);
}

TreeNode *makeRoomInNode(TreeNode *node, int size)
{
    int capacity = nodeCapacity(size);
    if (size >= capacity) {
        int const oldBytes = nodeBytes(capacity);
        capacity = nodeCapacity(size + 1);
        int const newBytes = nodeBytes(capacity);
        TreeNode *const prevNode = node;
        node = realloc(prevNode, newBytes);
        if (node) {
            if (node != prevNode)
                nodeStatus.reallocCount++;
            nodeStatus.allocMem += newBytes - oldBytes;
        }
    }
    return node;
}

TreeNode **addChildNode(TreeNode **parentPtr, char ch)
{
    ch -= NODE_LOWEST_VALUE;
    TreeNode *parent = *parentPtr;
    parent = makeNodeWritable(parent);
    if (!parent)
        return NULL;
    uint32_t const childrenBits = parent->activeChildren;
    assert(!bitIsSet(childrenBits, ch));
    int const size = numberOfSetBits(childrenBits & ~NODE_ENDWORD_FLAG);
    parent = makeRoomInNode(parent, size);
    if (!parent)
        return NULL;
    *parentPtr = parent;
    TreeNode *child = &stockNode;
    nodeStatus.nodes++;
    int childIdx = numberOfSetBitsBelow(childrenBits, ch);
    TreeNode **childPtr = parent->children + childIdx;
    memmove(childPtr + 1, childPtr, (size - childIdx) * sizeof(TreeNode *));
    *childPtr = child;
    parent->activeChildren = childrenBits | (1 << ch);
    return childPtr;
}

TreeNode *findNode(TreeNode *node, const char *word)
{
    for (char ch = *word++; ch; ch = *word++) {
        TreeNode *nextNode = *getChildNode(node, ch);
        if (!nextNode)
            break;
        node = nextNode;
    }
    return node;
}

bool insertWord(TreeNode **node, const char *word)
{
    char lastCh = '\0';
    for (char ch = *word++; ch; ch = *word++) {
        if (!node)
            return false;
        if (!*node && !(*node = createNode()))
            return false;
        TreeNode **nextNode = getChildNode(*node, ch);
        if (nextNode)
            node = nextNode;
        else
            node = addChildNode(node, ch);
        lastCh = ch;
    }
    if (lastCh)
        setAnyNodeEndsWord(node);
    return true;
}

void autoComplete(TreeNode *node)
{
    TreeNode **child = node->children + 0;
    uint32_t activeChildren = node->activeChildren;
    for (char ch = 0; ch < NODE_VALUE_SPAN; ch++) {
        if (activeChildren & 1) {
            char value = ch + NODE_LOWEST_VALUE;
            printf("%c", value);
            if (nodeEndsWord(*child))
                printf("]");
            autoComplete(*child);
            ++child;
        }
        activeChildren >>= 1;
    }
    printf("\n");
}

void simpleTest(void)
{
    TreeNode *root = NULL;
    insertWord(&root, "car");
    insertWord(&root, "cat");
    insertWord(&root, "cart");

    const char prefix[] = "ca";
    printf("%s->\n", prefix);

    TreeNode *endOfPrefix = findNode(root, prefix);
    autoComplete(endOfPrefix);

    destroyNode(&root);
    nodeStatus.reallocCount = 0;
    assert(!nodeStatus.allocMem);
}

char *readLine(char *buf, int bufSize, FILE *f)
{
    char *line = fgets(buf, bufSize, f);
    if (!line)
        return NULL;

    while (isspace(*line))
        line++;
    char *end = line;
    while (isalpha(*end))
        end++;
    *end = '\0';
    return (end > line) ? line : NULL;
}

struct LookupResult
{
    TreeNode **nodes;
    char *words;
} typedef LookupResult;

void freeResult(LookupResult *r)
{
    free(r->nodes);
    free(r->words);
}

LookupResult lookup(TreeNode *root, const char *wordList, size_t wordListSize, int wordCount)
{
    LookupResult result;
    result.nodes = malloc(sizeof(TreeNode *) * wordCount);
    result.words = malloc(wordListSize);
    TreeNode **nodePtr = result.nodes;
    char *outWord = result.words;

    const char *word = wordList;
    while (wordCount--) {
        TreeNode *const node = findNode(root, word);
        assert(node);
        *nodePtr++ = node;
        int const len = strlen(word) + 1;
        memcpy(outWord, word, len);
        outWord += len;
        word += len;
    }
    return result;
}

void fileTest(const char *filename)
{
    size_t wordListSize = 0;

    TreeNode *root = NULL;
    FILE *fi = fopen(filename, "r");
    if (!fi)
        return;
    char buf[512];
    char *word;
    while ((word = readLine(buf, sizeof(buf), fi))) {
        wordListSize += strlen(word) + 1;
        insertWord(&root, word);
    }

    fseek(fi, 0, SEEK_SET);
    int wordCount = 0;
    char *wordList = malloc(wordListSize);
    char *listP = wordList;
    while ((word = readLine(buf, sizeof(buf), fi))) {
        int len = strlen(word);
        strcpy(listP, word);
        listP += len + 1;
        ++wordCount;
    }

    fclose(fi);
    printf("Total nodes: %zu\n", nodeStatus.nodes);
    printf("Nodes allocated: %zu\n", nodeStatus.allocNodes);
    printf("Memory allocated: %zu\n", nodeStatus.allocMem);
    printf("Node reallocations: %zu\n", nodeStatus.reallocCount);

    char *permutedWordList = permuteWordList(wordList, wordCount);
    free(wordList);

    LookupResult result = lookup(root, permutedWordList, wordListSize, wordCount);
    freeResult(&result);
    stopwatchReset();
    result = lookup(root, permutedWordList, wordListSize, wordCount);
    freeResult(&result);
    size_t ms = stopwatchElapsed();

    printf("Lookup took %zu ms\n", ms);
    free(permutedWordList);

    destroyNode(&root);
    assert(!nodeStatus.nodes);
    assert(!nodeStatus.allocNodes);
    assert(!nodeStatus.allocMem);
}

int main(void)
{
    setupStockNodes();
    simpleTest();
    fileTest("C:\\wc\\words_alpha.txt");
}
