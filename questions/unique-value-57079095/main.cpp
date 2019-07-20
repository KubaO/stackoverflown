// https://github.com/KubaO/stackoverflown/tree/master/questions/unique-value-57079095
#include <cstdint>
#include <map>
#include <memory>

struct no_copy {
   no_copy() = default;
   no_copy(const no_copy &) = delete;
   no_copy &operator=(const no_copy &) = delete;
};

template <typename T>
class cache_traits;
template <typename T>
class cache;

class cache_access {
  private:
   cache_access() = default;
public:
   cache_access(const cache_access &) = default;
   cache_access(cache_access &&) = default;
   template <typename, typename> friend class maker_t;
};

template <typename T>
class maker_base {
   // This class implements cache-friendlier dispatch. Instead of having an instance
   // with a pointer to virtual table, the instance itself is a "virtual table".
   // The users then refer to this instance, statically allocated, via a single
   // pointer/reference. On the flipside, good luck with a compiler devirtualizing this
   // stuff. But who knows. Maybe.
  public:
   struct private_constructor {
      template <typename, typename>
      friend class maker_t;

     private:
      private_constructor() = default;
   };

   using c_value_t = typename std::add_const_t<std::decay_t<T>>;
   using key_t = typename cache_traits<T>::key_type;
   using ptr_t = std::shared_ptr<c_value_t>;

  private:
   static ptr_t make_impl_m(cache<T> &, const maker_base<T> &, key_t &&) { return {}; }
   static ptr_t make_impl_c(cache<T> &, const maker_base<T> &, const key_t &) {
      return {};
   }
   decltype(&make_impl_m) make_m = make_impl_m;
   decltype(&make_impl_c) make_c = make_impl_c;

  public:
   maker_base() = delete;
   maker_base(decltype(&make_impl_m) m, decltype(&make_impl_c) c)
       : make_m(m), make_c(c) {}

   ptr_t make(cache<T> &c, key_t &&k) const {
      // This is effectively a virtual method, dispatched via make_m
      return make_m(c, *this, std::move(k));
   }
   ptr_t make(cache<T> &c, const key_t &k) const {
      // This is effectively a virtual method, dispatched via make_c
      return make_c(c, *this, k);
   }
};

template <typename T, typename U>
class maker_t : public maker_base<T> {
  public:
   using key_t = typename maker_base<T>::key_t;
   using ptr_t = typename maker_base<T>::ptr_t;

   maker_t() : maker_base<T>(&make_impl, &make_impl) {}
   static const maker_base<T> &fabricator() {
      static const maker_t<T, U> fabricator;
      return fabricator;
   }

  private:
   static ptr_t make_impl(cache<T> &cache, const maker_base<T> &maker, key_t &&key) {
      std::shared_ptr<T> ptr;
      cache_access token;
      if constexpr (std::is_move_constructible_v<key_t>)
         ptr = std::make_shared<U>(std::move(key), token);
      else
         ptr = std::make_shared<U>(key, token);
      ptr->set_cache(&cache, &maker);
      return ptr;
   }
   static ptr_t make_impl(cache<T> &cache, const maker_base<T> &maker, const key_t &key) {
      cache_access token;
      auto ptr = std::make_shared<U>(std::move(key), token);
      ptr->set_cache(&cache, &maker);
      return ptr;
   }
};

template <typename T>
class cached_value : no_copy {
  protected:
   cache<T> *m_cache = {};
   const maker_base<T> *m_maker = {};
   using key_t = typename maker_base<T>::key_t;
   using ptr_t = typename maker_base<T>::ptr_t;

   template <typename, typename>
   friend class maker_t;

   cached_value() = default;

   void set_cache(cache<T> *cache, const maker_base<T> *maker) {
      m_maker = maker;
      m_cache = cache;
   }

   template <typename U, typename... Args>
   static std::shared_ptr<U> make_shared(Args &&... args) {
      return std::make_shared<U>(std::forward<Args>(args)...);
   }

  public:
   using private_constructor = typename maker_base<T>::private_constructor;
   template <typename Key = key_t>
   std::enable_if_t<std::is_same_v<key_t, std::decay_t<Key>>, ptr_t> make(
       Key &&key = {}) const {
      return m_cache->make_cached(*m_maker, std::forward<Key>(key));
   }
};

template <typename T>
class cache {
   using c_value_t = typename maker_base<T>::c_value_t;
   using key_t = typename maker_base<T>::key_t;
   using ptr_t = typename maker_base<T>::ptr_t;
   std::map<key_t, std::weak_ptr<c_value_t>> store;
   typename decltype(store)::iterator prune_it;  // = store.begin();

  public:
   size_t size() const { return store.size(); }
   bool prune_step() {
      // std::map is an associative container, and its iterators remain
      // valid during modifications
      if (prune_it == store.end()) prune_it = store.begin();
      if (prune_it == store.end()) return false;
      if (prune_it->second.expired()) {
         prune_it = store.erase(prune_it);
         return true;
      }
      std::advance(prune_it, 1);
      return false;
   }
   void prune() {
      while (prune_step())
         ;
   }

   template <typename Key>
   std::enable_if_t<std::is_same_v<key_t, std::decay_t<Key>>, ptr_t> make_cached(
       const maker_base<T> &maker, Key &&key) {
      prune_step();
      auto it = store.find(key);
      if (it != store.end()) {
         auto ptr = it->second.lock();
         if (ptr) return ptr;
      }
      auto ptr = maker.make(*this, key);
      if (ptr) {
         auto const node = store.emplace(std::forward<Key>(key), ptr);
         assert(node.second);
      } else if (it != store.end()) {
         store.erase(it);
      }
      return ptr;
   }
   template <typename U, typename Key>
   ptr_t make_cached(Key &&key) {
      return make_cached(maker_t<T, U>::fabricator(), std::forward<Key>(key));
   }
   template <typename U>
   ptr_t make_cached() {
      return make_cached(maker_t<T, U>::fabricator(), key_t{});
   }
};

enum class Color { Red, Green, Blue };

struct Shape;

template <>
class cache_traits<Shape> {
  public:
   using key_type = Color;
   template <typename U, typename... Args>
   static key_type make_key(Color color) {
      return color;
   }
};

struct Shape : public cached_value<Shape> {
  protected:
   Shape() = default;
   template <typename, typename>
   friend class ::maker_t;
};

struct Circle : public Shape {
  public:
   Color c;
   Circle(Color c, cache_access) : c(c) {}
};

#include <iostream>

class Program {
   cache<Shape> c;

  public:
   int main() {
      {

         Circle cir{Color{}, {}};
         auto cR = c.make_cached<Circle>();
         // assert(cR.)
         auto cG = c.make_cached<Circle>(Color::Green);
         assert(c.size() == 2);
         assert(cR && cG);
         auto cB = cG->make(Color::Blue);
         assert(cB);
         assert(c.size() == 3);
         assert(cG->make({}) == cR);
         assert(c.make_cached<Circle>(Color::Green) == cG);
         assert(c.make_cached<Circle>(Color::Blue) == cB);
         assert(c.size() == 3);
         c.prune();
         assert(c.size() == 3);
      }
      assert(c.size() == 3);
      c.prune();
      assert(c.size() == 0);

      return 0;
   }
};

int main() {
   Program prog;
   return prog.main();
}
