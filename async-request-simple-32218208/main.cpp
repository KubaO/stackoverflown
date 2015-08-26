#include <QtCore>

class Request;
typedef QSharedPointer<Request> RequestPtr;
class Request : public QObject {
   Q_OBJECT
public:
   static QAtomicInt m_count;
   Request() { m_count.ref(); }
   ~Request() { m_count.deref(); }
   int taxIncrease;
   Q_SIGNAL void done(RequestPtr);
};
Q_DECLARE_METATYPE(RequestPtr)
QAtomicInt Request::m_count(0);

class Requester : public QObject {
   Q_OBJECT
   Q_PROPERTY (int catTax READ catTax WRITE setCatTax NOTIFY catTaxChanged)
   int m_catTax;
public:
   Requester(QObject * parent = 0) : QObject(parent), m_catTax(0) {}
   Q_SLOT int catTax() const { return m_catTax; }
   Q_SLOT void setCatTax(int t) {
      if (t != m_catTax) {
         m_catTax = t;
         emit catTaxChanged(t);
      }
   }
   Q_SIGNAL void catTaxChanged(int);
   Q_SIGNAL void hasRequest(RequestPtr);
   void sendNewRequest() {
      RequestPtr req(new Request);
      req->taxIncrease = 5;
      connect(req.data(), &Request::done, this, [this, req]{
         setCatTax(catTax() + req->taxIncrease);
         qDebug() << objectName() << "has cat tax" << catTax();
         QCoreApplication::quit();
      });
      emit hasRequest(req);
   }
};

class Processor : public QObject {
   Q_OBJECT
public:
   Q_SLOT void process(RequestPtr req) {
      QThread::msleep(50); // Pretend to do some work.
      req->taxIncrease --; // Figure we don't need so many cats after all...
      emit req->done(req);
      emit done(req);
   }
   Q_SIGNAL void done(RequestPtr);
};

struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
   struct C { ~C() { Q_ASSERT(Request::m_count == 0); } } check;
   QCoreApplication app(argc, argv);
   qRegisterMetaType<RequestPtr>();
   Processor processor;
   Thread thread;
   processor.moveToThread(&thread);
   thread.start();

   Requester requester1;
   requester1.setObjectName("requester1");
   QObject::connect(&requester1, &Requester::hasRequest, &processor, &Processor::process);
   requester1.sendNewRequest();
   {
      Requester requester2;
      requester2.setObjectName("requester2");
      QObject::connect(&requester2, &Requester::hasRequest, &processor, &Processor::process);
      requester2.sendNewRequest();
   } // requester2 is destructed here
   return app.exec();
}

#include "main.moc"

#if 0

In the example below, we create a requester object that is destructed immediately after the request has been placed. Since we can connect to functors, we can use a naked `QObject` and write slots using lambdas that execute in the naked object's thread context. That's a neat idiom that sometimes comes handy, I think.


#endif
