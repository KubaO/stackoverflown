// https://github.com/KubaO/stackoverflown/tree/master/questions/qfuture-immediate-50772976
#include <QtConcurrent>

template <typename T> QFuture<T> finishedFuture(const T &val) {
   QFutureInterface<T> fi;
   fi.reportFinished(&val);
   return QFuture<T>(&fi);
}

QFuture<bool> foo(bool val, bool valid) {
   if (!valid)
      return {};
   return finishedFuture(val);
}

int main() {
   Q_ASSERT(foo(true, true));
   Q_ASSERT(!foo(false, true));
   Q_ASSERT(foo(false, false).isCanceled());
   Q_ASSERT(foo(true, false).isCanceled());
}
