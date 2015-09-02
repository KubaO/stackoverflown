// main.cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main(int, char**)
{
    const char * home = getenv("HOME");
    if (home == NULL || strlen(home) == 0 || access(home, F_OK) == -1) abort();
    const char * subPath = "/work/dog.txt";
    char * path = (char*)malloc(strlen(home) + strlen(subPath) + 1);
    if (path == NULL) abort();
    strcpy(path, home);
    strcat(path, subPath);
    FILE *fp = fopen(path, "w");
    if (fp != NULL) {
        fprintf(fp, "timer, timer3, timer5, timer6, timer7");
        fclose(fp);
    }
    free(path);
}
