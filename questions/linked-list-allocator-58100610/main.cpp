// https://github.com/KubaO/stackoverflown/tree/master/questions/linked-list-allocator-58100610
#include <forward_list>
#include <iostream>
#include <sstream>
#include <vector>

using element_type = int;

template <typename allocator> using list_type = std::forward_list<element_type, allocator>;

template <typename allocator>
std::vector<list_type<allocator>> parse(std::istream &in, allocator alloc)
{
   using list_t = list_type<allocator>;
   std::vector<list_t> lists;
   element_type el;
   list_t *list = {};
   do {
      in >> el;
      if (in.good()) {
         if (!list) list = &lists.emplace_back(alloc);
         list->push_front(std::move(el));
      }
      while (in.good()) {
         int c = in.get();
         if (!isspace(c)) {
            in.unget();
            break;
         }
         else if (c=='\n') list = {};
      }
   } while (in.good() && !in.eof());
   for (auto &list : lists) list.reverse();
   return lists;
}

const std::vector<std::vector<element_type>> test_data = {
   {6, 18, 5, 20, 48, 2, 97},
   {3, 6, 9, 12, 28, 5, 7, 10}
};

template <typename allocator = std::allocator<element_type>>
void test(const std::string &str, allocator alloc = {})
{
   std::istringstream input{str};
   auto lists = parse(input, alloc);
   assert(lists.size() == 4);
   lists.erase(lists.begin()+2); // remove the 3rd list
   lists.erase(lists.begin()+0); // remove the 1st list
   for (int i = 0; i < test_data.size(); i++)
      assert(std::equal(test_data[i].begin(), test_data[i].end(), lists[i].begin()));
}

std::string generate_input()
{
   std::stringstream s;
   for (auto &data : test_data) {
      s << data.size() << "\n";
      for (const element_type &el : data) s << el << " ";
      s << "\n";
   }
   return s.str();
}

class segment_allocator_base
{
protected:
   static constexpr size_t segment_size = 128;
   using segment = std::vector<char>;
   struct free_node {
      free_node *next;
      free_node() = delete;
      free_node(const free_node &) = delete;
      free_node &operator=(const free_node &) = delete;
      free_node *stepped_by(size_t element_size, int n) const {
         auto *p = const_cast<free_node*>(this);
         return reinterpret_cast<free_node*>(reinterpret_cast<char*>(p) + (n * element_size));
      }
   };
   struct segment_store {
      size_t element_size;
      free_node *free = {};
      explicit segment_store(size_t element_size) : element_size(element_size) {}
      std::forward_list<segment> segments;
   };
   template <typename T> static constexpr size_t size_for() {
      constexpr size_t T_size = sizeof(T);
      constexpr size_t element_align = std::max(alignof(free_node), alignof(T));
      constexpr auto padding = T_size % element_align;
      return T_size + padding;
   }
   struct pimpl {
      std::vector<segment_store> stores;
      template <typename T> segment_store &store_for() {
         constexpr size_t element_size = size_for<T>();
         for (auto &s : stores)
            if (s.element_size == element_size) return s;
         return stores.emplace_back(element_size);
      }
   };
   std::shared_ptr<pimpl> dp{new pimpl};
};

template<typename T>
class segment_allocator : public segment_allocator_base
{
   segment_store *d = {};
   static constexpr size_t element_size = size_for<T>();
   static free_node *advanced(free_node *p, int n) { return p->stepped_by(element_size, n); }
   static free_node *&advance(free_node *&p, int n) { return (p = advanced(p, n)); }
   void mark_free(free_node *free_start, size_t n)
   {
      auto *p = free_start;
      for (; n; n--) p = (p->next = advanced(p, 1));
      advanced(p, -1)->next = d->free;
      d->free = free_start;
   }
public:
   using value_type = T;
   using pointer = T*;
   template <typename U> struct rebind {
      using other = segment_allocator<U>;
   };
   segment_allocator() : d(&dp->store_for<T>()) {}
   segment_allocator(segment_allocator &&o) = default;
   segment_allocator(const segment_allocator &o) = default;
   segment_allocator &operator=(const segment_allocator &o) {
      dp = o.dp;
      d = o.d;
      return *this;
   }
   template <typename U> segment_allocator(const segment_allocator<U> &o) :
      segment_allocator_base(o), d(&dp->store_for<T>()) {}
   pointer allocate(const size_t n) {
      // TODO: This code should be rewriteen to use operations that expose semantics of
      // it all, so that the comments below should not need to be written. As it is, it's yuck.
      if (n == 0) return {};
      if (d->free) {
         // look for a sufficiently long contiguous region
         auto **base_ref = &d->free;
         auto *base = *base_ref;
         do {
            auto *p = base;
            for (auto need = n; need; need--) {
               auto *const prev = p;
               auto *const next = prev->next;
               advance(p, 1);
               if (need > 1 && next != p) {
                  base_ref = &(prev->next);
                  base = next;
                  break;
               } else if (need == 1) {
                  *base_ref = next; // remove this region from the free list
                  return reinterpret_cast<pointer>(base);
               }
            }
         } while (base);
      }
      // generate a new segment, guaranteed to contain enough space
      size_t count = std::max(n, segment_size);
      auto &segment = d->segments.emplace_front(count);
      auto *const start = reinterpret_cast<free_node*>(segment.data());
      if (count > n)
         mark_free(advanced(start, n), count - n);
      else
         d->free = nullptr;
      return reinterpret_cast<pointer>(start);
   }
   void deallocate(pointer ptr, std::size_t n) {
      mark_free(reinterpret_cast<free_node*>(ptr), n);
   }

   using propagate_on_container_copy_assignment = std::true_type;
   using propagate_on_container_move_assignment = std::true_type;
};

int main()
{  
   auto test_input_str = generate_input();
   std::cout << test_input_str << std::endl;
   test(test_input_str);
   test<segment_allocator<element_type>>(test_input_str);
   return 0;
}
