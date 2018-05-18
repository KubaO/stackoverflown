// https://github.com/KubaO/stackoverflown/tree/master/questions/vector-nodefault-33380402
#include <QVector>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
using InputFieldIndex = int;

class InputField {
   using self = InputField;
   InputFieldIndex m_index;
   std::string m_name;
public:
   InputField() = delete;
   InputField(const InputField &) = default;
   InputField(InputField &&) = default;
   InputField& operator=(const InputField&) = default;
   InputField& operator=(InputField&&) = default;
   InputField(InputFieldIndex uiIndex, std::string name) :
      m_index(std::move(uiIndex)), m_name(std::move(name)) {}
};

#define QVECTOR_NON_DEFAULT_CONSTRUCTIBLE(Type) \
   template <> QVector<Type>::QVector(int) = delete; \
   template <> void QVector<Type>::resize(int newSize) { \
   Q_ASSERT(newSize <= size()); \
   detach(); \
   } \
   template <> void QVector<Type>::defaultConstruct(Type*, Type*) { Q_ASSERT(false); }

QVECTOR_NON_DEFAULT_CONSTRUCTIBLE(InputField)

template <typename T> class Wrapper final {
   union {
      T object;
   };
   bool no_object = false;
   void cond_destruct() {
      if (!no_object)
         object.~T();
      no_object = true;
   }
public:
   Wrapper() : no_object(true) {}
   Wrapper(const Wrapper &o) : no_object(o.no_object) {
      if (!no_object)
         new (&object) T(o.object);
   }
   Wrapper(Wrapper &&o) : no_object(o.no_object) {
      if (!no_object)
         new (&object) T(std::move(o.object));
   }
   Wrapper(const T &o) : object(o) {}
   Wrapper(T &&o) : object(std::move(o)) {}
   template <class...Args> Wrapper(Args...args) : object(std::forward<Args>(args)...) {}
   template <class U, class...Args> Wrapper(std::initializer_list<U> init, Args...args) :
      object(init, std::forward<Args>(args)...) {}
   operator T&      () &      { assert(!no_object); return object; }
   operator T&&     () &&     { assert(!no_object); return std::move(object); }
   operator T const&() const& { assert(!no_object); return object; }
   Wrapper &operator=(const Wrapper &o) & {
      if (no_object)
         ::new (&object) T(o);
      else
         object = o.object;
      no_object = false;
      return *this;
   }
   Wrapper &operator=(Wrapper &&o) & {
      if (no_object)
         ::new (&object) T(std::move(o.object));
      else
         object = std::move(o.object);
      no_object = false;
      return *this;
   }
   template<class... Args> T &emplace(Args&&... args) {
      cond_destruct();
      ::new (&object) T(std::forward<Args>(args)...);
      no_object = false;
      return object;
   }
   ~Wrapper() {
      cond_destruct();
   }
};

int main() {
   std::vector<InputField> v1, v2;
   v1.push_back({1, "2"});
   v2 = v1;
   QVector<InputField> v3, v4;
   v3.push_back({1, "2"});
   v4 = v3;
   QVector<Wrapper<InputField>> v5, v6;
   v5.push_back({1, "2"});
   v6 = v5;
   std::vector<Wrapper<InputField>> v7, v8;
   v7.push_back({1, "2"});
   v8 = v7;

   InputField f1{1, "2"};
   f1 = {2, "3"};

   using WInputField = Wrapper<InputField>;
   WInputField f2{1, "2"};
   f2 = {2, "3"};
   f2 = f1;
   f2 = const_cast<const InputField&>(f1);
   f2 = InputField{1, "2"};
   f2 = std::move(f1);
   f2.emplace(3, "4");

   InputField l1 = f1;
   InputField l2 = WInputField{1, "2"};
   InputField l3 = static_cast<const WInputField &>(WInputField{1, "2"});
}
