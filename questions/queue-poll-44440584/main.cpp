// https://github.com/KubaO/stackoverflown/tree/master/questions/queue-poll-44440584
#include <QtCore>
#include <private/qthread_p.h>
#include <climits>

uint postedEventsCountForThread(QThread * thread) {
   if (!thread)
      return -1;
   auto threadData = QThreadData::get2(thread);
   QMutexLocker lock(&threadData->postEventList.mutex);
   return threadData->postEventList.size() - threadData->postEventList.startOffset;
}

uint postedEventsCountFor(QObject * target) {
   return postedEventsCountForThread(target->thread());
}

uint qGlobalPostedEventsCount(); // exported in Qt but not declared
uint postedEventsCountForPublic(QObject * target, int timeout = 1000) {
   uint count = -1;
   QMutex mutex;
   struct Event : QEvent {
      QMutex & mutex;
      QMutexLocker lock;
      uint & count;
      Event(QMutex & mutex, uint & count) :
         QEvent(QEvent::None), mutex(mutex), lock(&mutex), count(count) {}
      ~Event() {
         count = qGlobalPostedEventsCount();
      }
   };
   QCoreApplication::postEvent(target, new Event(mutex, count), INT_MAX);
   if (mutex.tryLock(timeout)) {
      mutex.unlock();
      return count;
   }
   return -1;
}

int main(int argc, char ** argv) {
   QCoreApplication app(argc, argv);
   struct Receiver : QObject {
      bool event(QEvent *event) override {
         if (event->type() == QEvent::User)
            QThread::currentThread()->quit();
         return QObject::event(event);
      }
   } obj;
   struct Thread : QThread {
      QMutex mutex;
      Thread() { mutex.lock(); }
      void run() override {
         QMutexLocker lock(&mutex);
         QThread::run();
      }
   } thread;
   thread.start();
   obj.moveToThread(&thread);
   QCoreApplication::postEvent(&obj, new QEvent(QEvent::None));
   QCoreApplication::postEvent(&obj, new QEvent(QEvent::None));
   QCoreApplication::postEvent(&obj, new QEvent(QEvent::None));
   QCoreApplication::postEvent(&obj, new QEvent(QEvent::User));
   auto count1 = postedEventsCountFor(&obj);
   thread.mutex.unlock();
   auto count2 = postedEventsCountForPublic(&obj);
   thread.wait();
   auto count3 = postedEventsCountFor(&obj);
   Q_ASSERT(count1 == 4);
   Q_ASSERT(count2 == 4);
   Q_ASSERT(count3 == 0);
}
