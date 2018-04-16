// https://github.com/KubaO/stackoverflown/tree/master/questions/concurrent-combine-49802153
#include <QtConcurrent>
#include <functional>
#include <initializer_list>
#include <type_traits>

class Cls {
public:
   QString method1() const { return QStringLiteral("10"); }
   QString method2() const { return QStringLiteral("20"); }
   QString method3() const { return QStringLiteral("30"); }
};

template <class Method> struct apply_t {
   using result_type = typename std::result_of_t<Method()>;
   auto operator()(Method method) {
      return method();
   }
};

template <class Sequence, class A = apply_t<typename std::decay_t<Sequence>::value_type>>
A make_apply(Sequence &&) { return {}; }

template <class T> QVector<T> make_vector(std::initializer_list<T> init) {
   return {init};
}

int main() {
   Cls obj;
   auto const methods = make_vector({
                                       std::bind(&Cls::method1, &obj),
                                       std::bind(&Cls::method2, &obj),
                                       std::bind(&Cls::method3, &obj)
                                    });
   QFuture<QString> result =
         QtConcurrent::mapped(methods, make_apply(methods));
   Q_ASSERT((result.results() == QStringList{"10", "20", "30"}));
}

// See code in https://stackoverflow.com/q/43560492/1329652
#include <utility>
#include <type_traits>

template <class T> struct function_traits : function_traits<decltype(&T::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
   // specialization for pointers to member function
   using result_type = ReturnType;
};

template <class Callable>
struct CallableWrapper : Callable, function_traits<Callable> {
   CallableWrapper(const Callable &f) : Callable(f) {}
   CallableWrapper(Callable &&f) : Callable(std::move(f)) {}
};

template <class F> auto wrap(F &&f) {
   return CallableWrapper<F>(std::forward<F>(f));
}
// end function_traits code

#include <boost/range/irange.hpp>

template <class Object, class Sequence>
auto make_switcher(Object *object, Sequence &&methods) {
   auto fun = [object, methods = std::forward<Sequence>(methods)](const int &i){
      return (*object.*methods[i])();
   };
   return wrap(std::move(fun));
}

template <class Object, class Method>
auto make_switcher(Object *object, std::initializer_list<Method> methods) {
   return make_switcher(object, make_vector(methods));
}

void testSwitcher() {
   Cls obj;
   auto const methods = make_vector({ &Cls::method1, &Cls::method2, &Cls::method3 });
   auto s = make_switcher(&obj, methods);
   Q_ASSERT(s(0) == "10");
   Q_ASSERT(s(1) == "20");
   Q_ASSERT(s(2) == "30");
   auto range = boost::irange(0, 3);
   QFuture<QString> result = QtConcurrent::mapped(range.begin(), range.end(), s);
   Q_ASSERT((result.results() == QStringList{"10", "20", "30"}));
}

//

template <class Method, class Object>
auto wrap(Object *object) {
   auto methodCaller = [object](Method method){ return (*object.*method)(); };
   return wrap(std::move(methodCaller));
}

template <class Container, class Object>
auto wrap(Object *object, const Container &, typename Container::value_type* = {}) {
   return wrap<typename std::decay_t<Container>::value_type>(object);
}

template <class Method, class Object>
auto wrap(Object *object, Method,
          typename std::enable_if_t<std::is_member_function_pointer<Method>::value, bool> = {}) {
   return wrap<Method>(object);
}

void testWrap() {
   Cls obj;
   auto const methods = make_vector({ &Cls::method1, &Cls::method2, &Cls::method3 });
   QFuture<QString> result = QtConcurrent::mapped(methods, wrap(&obj, methods));
   Q_ASSERT((result.results() == QStringList{"10", "20", "30"}));
}
