// https://github.com/KubaO/stackoverflown/tree/master/questions/qtimer-retrofit-26713879
#include <QtCore>

struct Class : QObject {
   void TestFunc();
   void Log(const char *str) { qDebug() << str; }
};

#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
namespace detail { using QT_PREPEND_NAMESPACE(QTimer); }
#else
QT_BEGIN_NAMESPACE
Q_CORE_EXPORT void qDeleteInEventHandler(QObject *o);
QT_END_NAMESPACE
namespace detail {
template <class Fun> struct SingleShotHelper : QObject, Fun {
   QBasicTimer timer;
   template <class F> SingleShotHelper(int msec, QObject *context, F &&fun) :
      QObject(context ? context : QAbstractEventDispatcher::instance()),
      Fun(std::forward<F>(fun)) {
      timer.start(msec, this);
      if (!context)
         connect(qApp, &QCoreApplication::aboutToQuit, this, &QObject::deleteLater);
   }
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() != timer.timerId()) return;
      timer.stop();
      (*this)();
      QT_PREPEND_NAMESPACE(qDeleteInEventHandler)(this);
   }
};
class QTimer {
public:
   template <class Fun>
   inline static void singleShot(int msec, QObject *context, Fun &&fun) {
      new SingleShotHelper<Fun>(msec, context, std::forward<Fun>(fun));
   }
}; }
#endif

void Class::TestFunc() {
   detail::QTimer::singleShot(1000, this, [this]{
      emit Log("Timeout...");
      TestFunc();
   });
}

int main() {}
