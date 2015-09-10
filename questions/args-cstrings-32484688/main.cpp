// https://github.com/KubaO/stackoverflown/tree/master/questions/args-cstrings-32484688
#include <initializer_list>
#include <type_traits>
#include <cstdlib>
#include <cassert>
#include <vector>

class Args {
   struct cstr_vector : std::vector<char*> {
      ~cstr_vector() { for (auto str : *this) free(str); }
   } m_data;
   void append_copy(const char * s) {
      assert(s);
      auto copy = strdup(s);
      if (copy) m_data.push_back(copy); else throw std::bad_alloc();
   }
public:
   Args(std::initializer_list<const char*> l) {
      for (auto str : l) append_copy(str);
      m_data.push_back(nullptr);
   }
   template <std::size_t N>
   Args(const char * const (&l)[N]) {
      for (auto str : l) append_copy(str);
      m_data.push_back(nullptr);
   }
   /// Initializes from a null-terminated array of strings.
   template<class C, typename = typename std::enable_if<std::is_same<C, char const**>::value>::type>
   Args(C l) {
      while (*l) append_copy(*l++);
      m_data.push_back(nullptr);
   }
   /// Initializes from an array of strings with a given number of elements.
   Args(const char ** l, size_t count) {
      while (count--) append_copy(*l++);
      m_data.push_back(nullptr);
   }
   Args(Args && o) = default;
   Args(const Args &) = delete;
   size_t size() const { return m_data.size() - 1; }
   char ** data() { return m_data.data(); }
   bool operator==(const Args & o) const {
      if (size() != o.size()) return false;
      for (size_t i = 0; i < size(); ++i)
         if (strcmp(m_data[i], o.m_data[i]) != 0) return false;
      return true;
   }
};

#include <iostream>

extern "C" int gsapi_init_with_args(void*, int argc, char** argv) {
   for (int i = 0; i < argc; ++i)
      std::cout << "arg " << i << "=" << argv[i] << std::endl;
   return 0;
}

int main()
{
   Args args1 { "foo", "bar", "baz" };
   const char * args2i[] { "foo", "bar", "baz", nullptr };
   Args args2 { (const char **)args2i };
   const char * args3i[] { "foo", "bar", "baz" };
   Args args3 { args3i };
   const char * const args4i[] { "foo", "bar", "baz" };
   Args args4 { args4i };
   const char * args5i[] { "foo", "bar", "baz" };
   Args args5 { args5i, sizeof(args5i)/sizeof(args5i[0]) };

   assert(args1 == args2);
   assert(args2 == args3);
   assert(args3 == args4);
   assert(args4 == args5);

   gsapi_init_with_args(nullptr, args1.size(), args1.data());
}
