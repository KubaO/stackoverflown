// https://github.com/KubaO/stackoverflown/tree/master/questions/letter-count-56498637
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

template <typename T>
void saturating_inc(T &val) {
   if (val < std::numeric_limits<T>::max()) val++;
}

template <typename T, T min, T max>
class Histogram {
   using counter_type = unsigned;
   using storage_type = std::vector<counter_type>;
   storage_type counts;

  public:
   template <typename U>
   void count(U val) {
      if (val >= min && val <= max) saturating_inc(counts[size_t(val - min)]);
   }
   Histogram() : counts(1 + max - min) {}
   struct element {
      T value;
      counter_type count;
   };

   class const_iterator {
      T val;
      storage_type::const_iterator it;

     public:
      const_iterator(T val, storage_type::const_iterator it) : val(val), it(it) {}
      const_iterator &operator++() {
         ++val;
         ++it;
         return *this;
      }
      bool operator!=(const const_iterator &o) const { return it != o.it; }
      element operator*() const { return {val, *it}; }
   };
   const_iterator begin() const { return {min, counts.begin()}; }
   const_iterator end() const { return {0, counts.end()}; }
};

template <class C, class T>
class istream_range {
   C &ref;

  public:
   istream_range(C &ref) : ref(ref) {}
   std::istream_iterator<T> begin() { return {ref}; }
   std::istream_iterator<T> end() { return {}; }
};

template <class T, class C>
istream_range<C, T> make_range(C &ref) {
   return {ref};
}

int main() {
   Histogram<char, 'a', 'z'> counts;

   std::ifstream file;
   file.open("words_alpha.txt");

   for (auto ch : make_range<char>(file)) counts.count(tolower(ch));

   for (auto c : std::as_const(counts)) std::cout << c.value << ' ' << c.count << '\n';
}
