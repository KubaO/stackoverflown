#include <map>
#include <list>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <iostream>

struct A
{
    int i;
    double d;
    std::string s;
};

std::pair<std::string, A> pairify(const A& a) {
    return std::make_pair(a.s, a);
}

int main()
{
    std::list<A> list;
    std::map<std::string, A> map;

    std::transform(list.begin(), list.end(), std::inserter(map, map.end()), pairify);

    return 0;
}
