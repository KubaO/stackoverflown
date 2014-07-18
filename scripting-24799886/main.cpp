#include <QCoreApplication>
#include <QScriptEngine>
#include <QThread>

class ScriptEngine : public QScriptEngine {
   Q_OBJECT
   Q_SIGNAL void evaluateSignal(const QString &);
public:
   Q_SLOT void evaluate(const QString & str) { QScriptEngine::evaluate(str); }
   /// A thread-safe evaluate()
   Q_SLOT void safeEvaluate(const QString & str) { emit evaluateSignal(str); }
   explicit ScriptEngine(QObject * parent = 0) : QScriptEngine(parent) {
      connect(this, &ScriptEngine::evaluateSignal, this, &ScriptEngine::evaluate);
   }
};

class Thread : public QThread {
   // A thread that's safe to destruct, like it ought to be
   using QThread::run; // final
public:
   ~Thread() { quit(); wait(); }
};

int main(int argc, char ** argv) {
  QCoreApplication app(argc, argv);
  ScriptEngine engine;
  Thread worker;
  engine.globalObject().setProperty("qApp", engine.newQObject(qApp));
  engine.moveToThread(&worker);
  worker.start();

  QMetaObject::invokeMethod(&engine, "evaluate", Q_ARG(QString, "print('Hi!')"));
  engine.safeEvaluate("print('And hello!')");
  engine.safeEvaluate("qApp.quit()");
  return app.exec();
}

#include "main.moc"
