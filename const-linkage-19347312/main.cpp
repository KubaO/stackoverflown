#include "main.h"
#include <iostream>

void second();

int main(int, char **)
{
    std::cout << LABEL() << ARRAY()[0] << REQUEST_TIMEOUT_MS << std::endl;
    second();
}
