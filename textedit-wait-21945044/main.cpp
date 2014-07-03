#include <QApplication>
#include <QTextEdit>
#include <QBasicTimer>
#include <QSqlDatabase>
#include <QThread>

class ProductData {
};
Q_DECLARE_METATYPE(ProductData)

class PoS : public QWidget {
   Q_OBJECT
   enum QueryBehavior { FinalQuery, MultipleQuery };
   QBasicTimer m_queryTimer;
   QueryBehavior m_queryBehavior;
   Q_SLOT void on_lineEdit_textEdited() {
      if (m_queryBehavior == FinalQuery || !m_queryTimer.isActive())
         m_queryTimer.start(100, this);
   }
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_queryTimer.timerId()) return;
      m_queryTimer.stop();
      emit queryRequest();
   }
public:
   Q_SIGNAL void queryRequest();
   Q_SLOT void queryResponse(const ProductData &) { /* ... */ }
   // ...
};

class QueryExecutor : public QObject {
   Q_OBJECT
   QSqlDatabase m_dbConnection;
public:
   Q_SLOT void queryRequest() {
      if (!m_dbConnection.isOpen()) {
         // Open the database connection here, NOT in the constructor.
         // The constructor executes in the wrong thread.
         // ...
      }
      ProductData pdata;
      // ...
      emit queryResponse(pdata);
   }
   Q_SIGNAL void queryResponse(const ProductData &);
};

//! A thread that's always safe to destruct.
class Thread : public QThread {
private:
   using QThread::run; // This is a final class.
public:
   Thread(QObject * parent = 0) : QThread(parent) {}
   ~Thread() { quit(); wait(); }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   qRegisterMetaType<ProductData>();
   PoS pos;
   QueryExecutor executor;
   Thread thread; // must come after the executor!
   thread.start();
   executor.moveToThread(&thread);
   executor.connect(&pos, SIGNAL(queryRequest()), SLOT(queryRequest()));
   pos.connect(&executor, SIGNAL(queryResponse(ProductData)), SLOT(queryResponse(ProductData)));
   pos.show();
   return app.exec();
}

#include "main.moc"
