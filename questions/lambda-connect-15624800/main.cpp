// https://github.com/KubaO/stackoverflown/tree/master/questions/lambda-connect-15624800
#include <QtCore>

namespace LambdaHelper {
static constexpr bool const deferDelete = true;
bool canDefer() { return deferDelete && QAbstractEventDispatcher::instance(); }

class Base : public QObject {
   Q_OBJECT
protected:
   Q_SIGNAL void through();
   Q_SLOT virtual void call() = 0;
   void disconnectNotify(const QMetaMethod &signal) override {
      static auto const through = QMetaMethod::fromSignal(&Base::through);
      if (!signal.isValid() || signal == through) {
         if (canDefer())
            deleteLater();
         else
            delete this;
      }
   }
public:
   using QObject::QObject;
};

template <class Fun> class Functor final : public Base, public Fun {
   template <class F>
   friend QMetaObject::Connection connect(QObject*, const char*, F &&, QPointer<QObject> *);
   void call() override { (*this)(); }
   template <class F> Functor(QObject *parent, F &&fun) :
      Base(parent), Fun(std::forward<F>(fun)) {}
};

template <class F>
static QMetaObject::Connection connect(QObject *src, const char *signal, F &&fun, QPointer<QObject> *impl = {}) {
   Q_ASSERT(src); Q_ASSERT(signal);
   auto * const functor = new Functor<typename std::decay<F>::type>(src, std::forward<F>(fun));
   if (impl) *impl = functor;
   if (QObject::connect(src, signal, functor, SIGNAL(through())))
      return QObject::connect(functor, SIGNAL(through()), functor, SLOT(call()));
   if (impl) *impl = 0;
   delete functor;
   return {};
}
} // LambdaHelper

void execTick() {
   if (!LambdaHelper::canDefer() || !qApp) return;
   QTimer::singleShot(0, qApp, &QCoreApplication::quit);
   qApp->exec();
}

void test() {
   int ctr = 0;
   QPointer<QObject> monitor;
   {  // Test lambda invocation
      QObject o;
      LambdaHelper::connect(&o, SIGNAL(destroyed(QObject*)), [&]{ ctr++; }, &monitor);
      Q_ASSERT(monitor);
   }
   execTick();
   Q_ASSERT(ctr == 1);
   Q_ASSERT(!monitor);
   ctr = 0;
   {  // Test disconnection
      QObject o;
      auto conn = LambdaHelper::connect(&o, SIGNAL(destroyed(QObject*)), [&]{ ctr++; }, &monitor);
      Q_ASSERT(monitor);
      QObject::disconnect(conn);
      execTick();
      Q_ASSERT(!ctr);
      Q_ASSERT(!monitor);
   }
   Q_ASSERT(!ctr);
}

int main(int argc, char **argv) {
   Q_ASSERT(! LambdaHelper::canDefer());
   test();
   QCoreApplication app(argc, argv);
   Q_ASSERT(LambdaHelper::canDefer());
   test();
}

#include "main.moc"
