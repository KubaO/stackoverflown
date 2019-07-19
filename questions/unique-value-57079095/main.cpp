// https://github.com/KubaO/stackoverflown/tree/master/questions/unique-value-57079095
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <vector>

struct Shape;

template <typename T>
struct tag_t {};

class no_copy {
  public:
   no_copy() = default;
   no_copy(const no_copy &) = delete;
   no_copy(no_copy &&) = delete;
   no_copy &operator=(const no_copy &) = delete;
};

template <typename T>
class cache_traits;
template <typename T>
class cache;

template <typename T>
class maker_base {
  public:
   using c_value_t = typename std::add_const_t<std::decay_t<T>>;
   using key_t = typename cache_traits<T>::key_type;
   using ptr_t = std::shared_ptr<c_value_t>;

   virtual ptr_t make(cache<T> &cache, key_t &&) const { return {}; }
   virtual ptr_t make(cache<T> &cache, const key_t &) const { return {}; }
   virtual ~maker_base() = default;
};

template <typename T, typename U>
class maker_t : public maker_base<T> {
  public:
   using base = maker_base<T>;
   using key_t = typename base::key_t;
   using ptr_t = typename base::ptr_t;

   ptr_t make(cache<T> &cache, key_t &&key) const override {
      if constexpr (std::is_move_constructible_v<key_t>)
         return std::make_shared<U>(cache, std::move(key));
      else
         return std::make_shared<U>(cache, key);
   }
   ptr_t make(cache<T> &cache, const key_t &key) const override {
      return std::make_shared<U>(cache, std::move(key));
   }
   static const base &fabricator() {
      static const maker_t<T,U> fabricator;
      return fabricator;
   }
};

template <typename T>
class cached_value : no_copy {
  protected:
   cache<T> &m_cache;
   const maker_base<T> &m_maker;
   using key_t = typename maker_base<T>::key_t;
   using ptr_t = typename maker_base<T>::ptr_t;

   cached_value() = delete;

   template <typename U>
   friend class cache;

   template <typename U>
   cached_value(cache<T> &cache, tag_t<U>)
       : m_cache(cache), m_maker(maker_t<T, U>::fabricator()) {}

  public:
   template <typename Key>
   std::enable_if<std::is_same_v<key_t, std::decay_t<Key>>, ptr_t> make(
       Key &&key = key_t{}) const {
      return m_cache.make_cached(m_maker, std::forward<Key>(key));
   }
};

template <typename T>
class cache {
   using c_value_t = typename maker_base<T>::c_value_t;
   using key_t = typename maker_base<T>::key_t;
   using ptr_t = typename maker_base<T>::ptr_t;
   std::map<key_t, std::weak_ptr<c_value_t>> store;

  public:
   size_t size() const { return store.size(); }

   template <typename Key>
   std::enable_if_t<std::is_same_v<key_t, std::decay_t<Key>>, ptr_t> make_cached(
       const maker_base<T> &maker, Key &&key) {
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
   template <typename U>
   explicit Shape(cache<Shape> &cache, tag_t<U>) : cached_value(cache, tag_t<U>{}) {}
};

struct Circle : public Shape {
   Color c = Color::Red;
   Circle(cache<Shape> &cache, Color c) : Shape(cache, tag_t<Circle>{}) {}
};

#include <iostream>

class Program {
   cache<Shape> c;

  public:
   int main() {
      auto cR = c.make_cached<Circle>();
      auto cG = c.make_cached<Circle>(Color::Green);
      auto cB = c.make_cached<Circle>(Color::Blue);
      std::cout << c.size() << std::endl;
      assert(c.size() == 3);
      assert(cR && cG && cB);
      assert(c.make_cached<Circle>() == cR);
      assert(c.make_cached<Circle>(Color::Green) == cG);
      assert(c.make_cached<Circle>(Color::Blue) == cB);

      return 0;
   }
};

int main() {
   Program prog;
   return prog.main();
}
