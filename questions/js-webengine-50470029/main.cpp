#include <QtWebEngineWidgets>

void runScript(QWebEnginePage *page, const QWebEngineScript &script,
               const QWebEngineCallback<const QVariant &> &callback = {}) {
   page->runJavaScript(script.sourceCode(), script.worldId(), callback);
}

class Processor : public QObject {
   Q_OBJECT
   Q_SIGNAL void signal(const QVariant &);
protected:
   void onData(const QVariant &data) {
      qDebug() << data;
      qApp->exit();
   }
public:
   Processor(QObject *parent = {}) : QObject(parent) {
      connect(this, &Processor::signal, this, &Processor::onData, Qt::QueuedConnection);
   }
   void processData(const QVariant &data) {
      Q_EMIT signal(data);
   }
};

int main(int argc, char *argv[]) {
   QGuiApplication app(argc, argv);
   //QtWebEngineCore::in
   QWebEngineScript script;
   script.setWorldId(QWebEngineScript::ApplicationWorld);
   script.setSourceCode(R"( return "A!" )");
   QWebEnginePage page;
   Processor proc;
   runScript(&page, script, [proc = QPointer<Processor>(&proc)](auto &data){
      proc->processData(data);
   });
   return app.exec();
}
#include "main.moc"
