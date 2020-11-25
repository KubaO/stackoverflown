#include <chrono>
#include <vector>
#include <random>

extern "C" {

static std::chrono::time_point<std::chrono::steady_clock> reference;

void stopwatchReset()
{
    reference = std::chrono::steady_clock::now();
}

size_t stopwatchElapsed()
{
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - reference).count();
}

char *permuteWordList(char *words, size_t wordCount)
{
    size_t size = 0;
    std::vector<char *> wordLookup;
    while (wordCount--) {
        wordLookup.push_back(words);
        int const len = strlen(words) + 1;
        size += len;
        words += len;
    }

    //std::random_device rd;
    std::mt19937 g(1);
    std::shuffle(wordLookup.begin(), wordLookup.end(), g);

    char *const permuted = (char *) malloc(size);
    char *out = permuted;
    for (char *word : wordLookup) {
        int const len = strlen(word);
        strcpy(out, word);
        out += len + 1;
    }
    return permuted;
}

}
