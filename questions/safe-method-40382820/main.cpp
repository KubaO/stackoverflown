// https://github.com/KubaO/stackoverflown/tree/master/questions/safe-method-40382820
#include <QtCore>
#include <tuple>

bool isSafe(QObject * obj) {
   Q_ASSERT(obj->thread() || qApp && qApp->thread() == QThread::currentThread());
   auto thread = obj->thread() ? obj->thread() : qApp->thread();
   return thread == QThread::currentThread();
}

// Approach 1

template <typename Fun> void postCall(QObject * obj, Fun && fun) {
   qDebug() << __FUNCTION__;
   struct Event : public QEvent {
      using F = typename std::decay<Fun>::type;
      F fun;
      Event(F && fun) : QEvent(QEvent::None), fun(std::move(fun)) {}
      Event(const F & fun) : QEvent(QEvent::None), fun(fun) {}
      ~Event() { fun(); }
   };
   QCoreApplication::postEvent(
            obj->thread() ? obj : qApp, new Event(std::forward<Fun>(fun)));
}

// Approach 2

template <typename Class, typename... Args>
struct CallEvent : public QEvent {
   // See http://stackoverflow.com/a/7858971/1329652
   // See also http://stackoverflow.com/a/15338881/1329652
   template <int ...> struct seq {};
   template <int N, int... S> struct gens { using type = typename gens<N-1, N-1, S...>::type; };
   template <int ...S>        struct gens<0, S...> { using type = seq<S...>; };
   template <int ...S>        void callFunc(seq<S...>) { (obj->*method)(std::get<S>(args)...); }
   Class * obj;
   void (Class::*method)(Args...);
   std::tuple<typename std::decay<Args>::type...> args;
   CallEvent(Class * obj, void (Class::*method)(Args...), Args&&... args) :
      QEvent(QEvent::None), obj(obj), method(method), args(std::move<Args>(args)...) {}
   ~CallEvent() { callFunc(typename gens<sizeof...(Args)>::type()); }
};

template <typename Class, typename... Args> void postCall(Class * obj, void (Class::*method)(Args...), Args&& ...args) {
   qDebug() << __FUNCTION__;
   QCoreApplication::postEvent(
            obj->thread() ? static_cast<QObject*>(obj) : qApp, new CallEvent<Class, Args...>{obj, method, std::forward<Args>(args)...});
}

struct Class : QObject {
   int num{};
   QString str;
   void method1(int val) {
      if (!isSafe(this))
         return postCall(this, [=]{ method1(val); });
      qDebug() << __FUNCTION__;
      num = val;
   }
   void method2(const QString &val) {
      if (!isSafe(this))
         return postCall(this, &Class::method2, val);
      qDebug() << __FUNCTION__;
      str = val;
   }
};

class Thread : public QThread {
public:
   Thread(QObject * parent = nullptr) : QThread(parent) {}
   ~Thread() { quit(); wait(); }
};

void moveToOwnThread(QObject * obj) {
   Q_ASSERT(obj->thread() == QThread::currentThread());
   auto thread = new Thread{obj};
   thread->start();
   obj->moveToThread(thread);
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   Class c;
   moveToOwnThread(&c);

   const auto num = 44;
   const auto str = QString::fromLatin1("Foo");
   c.method1(num);
   c.method2(str);
   postCall(&c, [&]{ c.thread()->quit(); });
   c.thread()->wait();
   Q_ASSERT(c.num == num && c.str == str);
}
