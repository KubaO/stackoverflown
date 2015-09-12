// https://github.com/KubaO/stackoverflown/tree/master/questions/write-bundle-con-32533822
#include <fstream>

int main() {
    std::ofstream{"DienstplanerDB.sqlite"} << "Ooops" << std::endl;
}

