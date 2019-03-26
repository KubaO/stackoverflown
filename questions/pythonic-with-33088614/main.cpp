// https://github.com/KubaO/stackoverflown/tree/master/questions/pythonic-with-33088614
#include <cassert>
#include <cstdio>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>

template <typename T>
class context_manager_base {
  protected:
   std::optional<T> context;

  public:
   T &get() { return context.value(); }

   template <typename... Ts>
   std::enable_if_t<std::is_constructible_v<T, Ts...>, bool> enter(Ts &&... args) {
      context.emplace(std::forward<Ts>(args)...);
      return true;
   }
   bool exit(std::exception_ptr) {
      context.reset();
      return true;
   }
};

template <typename T>
class context_manager : public context_manager_base<T> {};

static class else_t *else_;
class pass_exceptions_t {};

void test() {
   /*
    * with<a>() >> with<b>() >= []{ };
    *
    *               >=
    *         >>    ,     []{}
    * with<a>(),with<b>()
   */
}

template <typename T> struct with1;

template <typename... Ts> struct with_list {
   std::tuple<Ts...> params;
   template <typename U> with_list<Ts..., U> operator>>(with1<U> &) {}
   template <typename F> with_list operator>=(F&&f) {
      std::apply(f, params);
   }
};

template <typename T> struct with1 {
   template<typename U> with_list<T, U> operator>>(with1<U> &) { return {this}; }
};

template <typename T>
class with {
   context_manager<T> mgr;
   bool ok;
   enum class Stage { WITH, ELSE, DONE } stage = Stage::WITH;
   std::exception_ptr exception = {};

  public:
   with(const with &) = delete;
   with(with &&) = delete;
   template <typename... Ts>
   explicit with(Ts &&... args) {
      try {
         ok = mgr.enter(std::forward<Ts>(args)...);
      } catch (...) {
         ok = false;
      }
   }
   template <typename... Ts>
   explicit with(pass_exceptions_t, Ts &&... args) {
      ok = mgr.enter(std::forward<Ts>(args)...);
   }
   ~with() {
      if (!mgr.exit(exception) && exception) std::rethrow_exception(exception);
   }
   with &operator>=(else_t *) {
      assert(stage == Stage::ELSE);
      return *this;
   }
   template <typename Fn>
   std::enable_if_t<std::is_invocable_r_v<void, Fn, decltype(mgr.get())>, with &>
   operator>=(Fn &&fn) {
      assert(stage == Stage::WITH);
      if (ok) try {
            std::forward<Fn>(fn)(mgr.get());
         } catch (...) {
            exception = std::current_exception();
         }
      stage = Stage::ELSE;
      return *this;
   }
   template <typename Fn>
   std::enable_if_t<std::is_invocable_r_v<bool, Fn, decltype(mgr.get())>, with &>
   operator>=(Fn &&fn) {
      assert(stage == Stage::WITH);
      if (ok) try {
            ok = std::forward<Fn>(fn)(mgr.get());
         } catch (...) {
            exception = std::current_exception();
         }
      stage = Stage::ELSE;
      return *this;
   }
   template <typename Fn>
   std::enable_if_t<std::is_invocable_r_v<void, Fn>, with &> operator>=(Fn &&fn) {
      assert(stage != Stage::DONE);
      if (stage == Stage::WITH) {
         if (ok) try {
               std::forward<Fn>(fn)();
            } catch (...) {
               exception = std::current_exception();
            }
         stage = Stage::ELSE;
      } else {
         assert(stage == Stage::ELSE);
         if (!ok) std::forward<Fn>(fn)();
         if (!mgr.exit(exception) && exception) std::rethrow_exception(exception);
         stage = Stage::DONE;
      }
      return *this;
   }
   template <typename Fn>
   std::enable_if_t<std::is_invocable_r_v<bool, Fn>, with &> operator>=(Fn &&fn) {
      assert(stage != Stage::DONE);
      if (stage == Stage::WITH) {
         if (ok) try {
               ok = std::forward<Fn>(fn)();
            } catch (...) {
               exception = std::current_exception();
            }
         stage = Stage::ELSE;
      } else {
         assert(stage == Stage::ELSE);
         if (!ok) std::forward<Fn>(fn)();
         if (!mgr.exit(exception) && exception) std::rethrow_exception(exception);
         stage = Stage::DONE;
      }
      return *this;
   }
};

class Resource {
   const std::string str;

  public:
   const bool successful;
   Resource(const Resource &) = delete;
   Resource(Resource &&) = delete;
   Resource(const std::string &str, bool succeed = true)
       : str(str), successful(succeed) {}
   void say(const std::string &s) {
      std::cout << "Resource(" << str << ") says: " << s << "\n";
   }
};

template <>
class context_manager<Resource> : public context_manager_base<Resource> {
  public:
   template <typename... Ts>
   bool enter(Ts &&... args) {
      context.emplace(std::forward<Ts>(args)...);
      return context.value().successful;
   }
};

template <>
class context_manager<std::FILE *> {
   std::FILE *file;

  public:
   std::FILE *get() { return file; }
   bool enter(const char *filename, const char *mode) {
      file = std::fopen(filename, mode);
      return file;
   }
   bool leave(std::exception_ptr) { return !file || (fclose(file) == 0); }
   ~context_manager() { leave({}); }
};

int main() {
   with<Resource>("foo"), with<Resource>("bar") >= []{};
   // with Resource("foo"):
   //   print("* Doing work!\n")
   with<Resource>("foo") >= [&] {
      std::cout << "1. Doing work\n";
   };

   // with Resource("foo", True) as r:
   //   r.say("* Doing work too")
   with<Resource>("bar", true) >= [&](auto &r) {
      r.say("2. Doing work too");
   };

   for (bool succeed : {true, false}) {
      // Shorthand for:
      // try:
      //   with Resource("bar", succeed) as r:
      //     r.say("Hello")
      //     print("* Doing work\n")
      // except:
      //   print("* Can't do work\n")
      with<Resource>("bar", succeed) >= [&](auto &r) {
         r.say("Hello");
         std::cout << "3. Doing work\n";
      } >= else_ >= [&] {
         std::cout << "4. Can't do work\n";
      };
   }
}
