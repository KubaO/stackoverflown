// https://github.com/KubaO/stackoverflown/tree/master/questions/shared-pointer-queued-49133331
#include <QtCore>

class Unique : public QObject {
   Q_OBJECT
   int const m_id = []{
      static QAtomicInteger<int> ctr;
      return ctr.fetchAndAddOrdered(1);
   }();
public:
   int id() const { return m_id; }
};

class IO : public QObject {
   Q_OBJECT
   int m_lastId = -1;
public:
   Q_SIGNAL void send(const QSharedPointer<Unique> &);
   Q_SLOT void receive(const QSharedPointer<Unique> & u) {
      m_lastId = u->id();
   }
   int lastId() const { return m_lastId; }
};

int main(int argc, char ** argv) {
   Q_ASSERT(QT_VERSION >= QT_VERSION_CHECK(5,9,0));
   QCoreApplication app{argc, argv};
   IO src, dst;
   QObject::connect(&src, &IO::send, &dst, &IO::receive, Qt::QueuedConnection);

   QSharedPointer<Unique> u;
   QWeakPointer<Unique> alive;
   int id = -1;

   // Single-threaded case
   alive = (u.reset(new Unique), u);
   id = u->id();
   Q_ASSERT(dst.lastId() != id); // the destination hasn't seen the object yet
   emit src.send(u);
   u.reset();
   Q_ASSERT(!u);                 // we gave up ownership of the object
   Q_ASSERT(dst.lastId() != id); // the destination mustn't seen the object yet
   Q_ASSERT(alive);              // the object must be still alive
   app.processEvents();
   Q_ASSERT(dst.lastId() == id); // the destination must have seen the object now
   Q_ASSERT(!alive);             // the object should have been destroyed by now

   // Multi-threaded setup
   struct Thread : QThread { ~Thread() { quit(); wait(); } } worker;
   worker.start();
   dst.moveToThread(&worker);
   QSemaphore s_src, s_dst;

   // This thread wins the race
   alive = (u.reset(new Unique), u);
   id = u->id();
   Q_ASSERT(dst.lastId() != id);
   QTimer::singleShot(0, &dst, [&]{ s_src.release(); s_dst.acquire(); });
                                 // stop the thread
   s_src.acquire();              // wait for thread to be stopped
   emit src.send(u);
   QTimer::singleShot(0, &dst, [&]{ s_src.release(); });
                                 // resume the main thread when done
   u.reset();
   Q_ASSERT(!u);
   Q_ASSERT(alive);              // we won the race: the object must be still alive
   s_dst.release();              // get the thread running
   s_src.acquire();              // wait for the thread to be done
   Q_ASSERT(dst.lastId() == id);
   Q_ASSERT(!alive);

   // The other thread wins the race
   alive = (u.reset(new Unique), u);
   id = u->id();
   Q_ASSERT(dst.lastId() != id);
   emit src.send(u);
   QTimer::singleShot(0, &dst, [&]{ s_src.release(); });
                                // resume the main thread when done
   u.reset();
   s_src.acquire();             // wait for worker thread to be done
   Q_ASSERT(!u);
   Q_ASSERT(!alive);            // we lost the race: the object must be gone
   Q_ASSERT(dst.lastId() == id); // yet the destination has received it!

   // Ensure the rendezvous logic didn't mess up
   Q_ASSERT(id == 2);
   Q_ASSERT(!s_src.available());
   Q_ASSERT(!s_dst.available());
}

#include "main.moc"
