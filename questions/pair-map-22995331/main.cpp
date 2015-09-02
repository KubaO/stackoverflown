#include <iostream>
#include <unordered_map>

template<> struct std::hash<std::pair<unsigned int, unsigned int>>
{
  typedef std::pair<unsigned int, unsigned int> argument_type;
  typedef std::size_t value_type;
  value_type operator()(argument_type const& s) const
  {
    value_type const h1 ( std::hash<unsigned int>()(s.first) );
    value_type const h2 ( std::hash<unsigned int>()(s.second) );
    return h1 ^ (h2 << 1);
  }
};

std::unordered_map<std::pair<unsigned int, unsigned int>, unsigned char> data;

int main() {
  using std::make_pair;
  data[make_pair(232432u, 234234u)] = 2;
  data[make_pair(2u, 3u)] = 1;
  std::cout << int(data[make_pair(232432u, 234234u)]) << std::endl;
  std::cout << int(data[make_pair(3u, 3u)]) << std::endl;
  std::cout << int(data[make_pair(232432u, 1u)]) << std::endl;
  std::cout << int(data[make_pair(2u, 3u)]) << std::endl;
}
