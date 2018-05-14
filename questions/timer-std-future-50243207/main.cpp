#include <QtCore>
#include <functional>
#include <future>
#include <cxxabi.h>


#if 0
template <class _Fp, class... _Args>
std::future<typename std::__invoke_of<typename std::decay<_Fp>::type, typename std::decay<_Args>::type...>::type>
async1(std::launch __policy, _Fp&& __f, _Args&&... __args) {
  qDebug() << typeid(_Fp).name();
}
#endif

void dump(const std::type_info &t) {
  int status = -1;
  qDebug() << abi::__cxa_demangle(t.name(), NULL, NULL, &status);
}

int main(int argc, char *argv[]) {
  struct Class {
    void func() {}
  } object;

  QCoreApplication app(argc, argv);

  auto funCall = std::bind(&Class::func, object);
  using funT = std::decay_t<decltype(funCall)>;
  using funR = std::future<std::result_of_t<funT()>>;
  funCall();
  /*
   * template <class _Fp, class... _Args>
future<typename __invoke_of<typename decay<_Fp>::type, typename decay<_Args>::type...>::type>
async(launch __policy, _Fp&& __f, _Args&&... __args)
*/
  dump(typeid(funT));
  dump(typeid(funR));

  dump(typeid(static_cast<funR(*)(std::launch, funT&)>(std::async)));
  qDebug() << (void*)static_cast<funR(*)(std::launch, funT&)>(std::async);

  auto f = static_cast<funR(*)(std::launch, funT&)>(std::async);
  f(std::launch::async, funCall);
  auto asyncCall = std::bind<std::decay_t<decltype(f)>, std::launch, decltype(funCall)>>(f, std::launch::async, funCall);
  asyncCall();
  //QTimer::singleShot(1000, asyncCall);

  return app.exec();
}
