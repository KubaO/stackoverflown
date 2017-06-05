// https://github.com/KubaO/stackoverflown/tree/master/questions/threaded-server-19280903
#include <QtNetwork>
#include <functional>

// A QThread that quits its event loop upon destruction,
// and waits for the loop to finish.
class StoppingThread : public QThread {
   Q_OBJECT
public:
   using QThread::QThread;
   ~StoppingThread() { quit(); wait(); qDebug() << this; }
};

// Deletes an object living in a thread upon thread's termination.
class ThreadedQObjectDeleter : public QObject {
   Q_OBJECT
   QPointer<QObject> m_object;
   ThreadedQObjectDeleter(QObject * object, QThread * thread) :
      QObject(thread), m_object(object) {}
   ~ThreadedQObjectDeleter() {
      if (m_object && m_object->thread() == 0)
         delete m_object;
   }
public:
   static void addDeleter(QObject * object, QThread * thread) {
      // The object must not be in the thread yet, otherwise we'd have
      // a race condition.
      Q_ASSERT(thread != object->thread());
      new ThreadedQObjectDeleter(object, thread);
   }
};

// Creates servers whenever the listening server gets a new connection
class ServerFactory : public QObject {
   Q_OBJECT
   std::function<QObject*(QTcpSocket*, QObject*)> m_factory;
public:
   template <typename T> struct Constructor {
      template <typename... Args> T* operator()(Args &&... args) {
         return new T(std::forward<Args>(args)...);
      }
   };
   template <typename F> ServerFactory(F && factory, QObject * parent = {}) :
      QObject(parent), m_factory(std::forward<F>(factory)) {}
   Q_SLOT void newConnection() {
      auto listeningServer = qobject_cast<QTcpServer*>(sender());
      if (!listeningServer) return;
      auto socket = listeningServer->nextPendingConnection();
      if (!socket) return;
      makeServerFor(socket);
   }
protected:
   virtual QObject * makeServerFor(QTcpSocket * socket) {
      auto server = m_factory(socket, {});
      socket->setParent(server);
      return server;
   }
};

// A server factory that makes servers in individual threads.
// The threads automatically delete itselves upon finishing.
// Destructing the thread also deletes the server.
class ThreadedServerFactory : public ServerFactory {
   Q_OBJECT
public:
   using ServerFactory::ServerFactory;
protected:
   QObject * makeServerFor(QTcpSocket * socket) override {
      auto server = ServerFactory::makeServerFor(socket);
      auto thread = new StoppingThread(this);
      connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
      connect(server, SIGNAL(destroyed()), thread, SLOT(quit()));
      ThreadedQObjectDeleter::addDeleter(server, thread);
      server->moveToThread(thread);
      thread->start();
      return server;
   }
};

// A telnet server with following functionality:
// 1. It echoes everything it receives,
// 2. It shows a smiley face upon receiving CR,
// 3. It quits the server upon ^C
// 4. It disconnects upon receiving 'Q'
class TelnetServer : public QObject {
   Q_OBJECT
   QTcpSocket * m_socket;
   bool m_firstInput = true;
   Q_SLOT void readyRead() {
      auto const data = m_socket->readAll();
      if (m_firstInput) {
         QTextStream(m_socket) << "Welcome from thread " << thread() << endl;
         m_firstInput = false;
      }
      for (auto c : data) {
         if (c == '\004') /* ^D */ { m_socket->close(); break; }
         if (c == 'Q') { QCoreApplication::exit(0); break; }
         m_socket->putChar(c);
         if (c == '\r') m_socket->write("\r\n:)", 4);
      }
      m_socket->flush();
   }
public:
   TelnetServer(QTcpSocket * socket, QObject * parent = {}) :
      QObject(parent), m_socket(socket)
   {
      connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
      connect(m_socket, SIGNAL(disconnected()), SLOT(deleteLater()));
   }
   ~TelnetServer() { qDebug() << this; }
};

int main(int argc, char *argv[])
{
   QCoreApplication app(argc,  argv);
   QTcpServer server;
   ThreadedServerFactory factory((ThreadedServerFactory::Constructor<TelnetServer>()));
   factory.connect(&server, SIGNAL(newConnection()), SLOT(newConnection()));
   server.listen(QHostAddress::Any, 8023);
   return app.exec();
}
#include "main.moc"
