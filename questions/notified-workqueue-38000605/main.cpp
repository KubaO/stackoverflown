// https://github.com/KubaO/stackoverflown/tree/master/questions/notified-workqueue-38000605
#include <QtCore>
#include <type_traits>

class WorkUnit;
class WorkQueue : public QObject {
   Q_OBJECT
   friend class WorkUnit;
   QThreadPool m_pool{this};
   union alignas(64) { // keep it in its own cache line
      QAtomicInt queuedUnits{0};
      char filler[64];
   } d;
   void isDone(WorkUnit * unit) {
      auto queued = d.queuedUnits.deref();
      emit workUnitDone(unit);
      if (!queued) emit finished();
   }
public:
   explicit WorkQueue(int initialUnits = 0) {
      if (initialUnits)
         QTimer::singleShot(0, [=]{
            for (int i = 0; i < initialUnits; ++i)
               emit workUnitDone(nullptr);
         });
   }
   Q_SLOT void addWork(WorkUnit * unit);
   template <typename F> void addFunctor(F && functor);
   Q_SIGNAL void workUnitDone(WorkUnit *);
   Q_SIGNAL void finished();
};

class WorkUnit : public QRunnable {
   friend class WorkQueue;
   WorkQueue * m_queue { nullptr };
   void run() override {
      work();
      m_queue->isDone(this);
   }
protected:
   virtual void work() = 0;
};

template <typename F>
class FunctorUnit : public WorkUnit, private F {
   void work() override { (*this)(); }
public:
   FunctorUnit(F && f) : F(std::move(f)) {}
};

void WorkQueue::addWork(WorkUnit *unit) {
   d.queuedUnits.ref();
   unit->m_queue = this;
   m_pool.start(unit);
}

template <typename F> void WorkQueue::addFunctor(F && functor) {
   addWork(new FunctorUnit<typename std::decay<F>::type>{std::forward<F>(functor)});
}

#include <random>

struct SleepyWork : WorkUnit {
   int usecs;
   SleepyWork(int usecs) : usecs(usecs) {}
   void work() override {
      QThread::usleep(usecs);
      qDebug() << "slept" << usecs;
   }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   std::random_device dev;
   std::default_random_engine eng{dev()};
   std::uniform_int_distribution<int> dist{1, 1000000};
   auto rand_usecs = [&]{ return dist(eng); };

   int workUnits = 50;
   WorkQueue queue{2*QThread::idealThreadCount()};
   QObject::connect(&queue, &WorkQueue::workUnitDone, [&]{
      if (workUnits) {
         if (workUnits % 2) {
            auto us = dist(eng);
            queue.addFunctor([us]{
               QThread::usleep(us);
               qDebug() << "slept" << us;
            });
         } else
            queue.addWork(new SleepyWork{rand_usecs()});
         --workUnits;
      }
   });
   QObject::connect(&queue, &WorkQueue::finished, [&]{
      if (workUnits == 0) app.quit();
   });

   return app.exec();
}

#include "main.moc"
