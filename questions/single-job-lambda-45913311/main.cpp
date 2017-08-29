// https://github.com/KubaO/stackoverflown/tree/master/questions/single-job-lambda-45913311
#include <QtWidgets>
#include <QtConcurrent>

class LogWindow : public QPlainTextEdit {
   Q_OBJECT
   QThreadPool m_pool;
   int g = {}; // can be accessed from the worker thread only
   void keyReleaseEvent(QKeyEvent * event) override {
      if (event->key() == Qt::Key_A)
         QtConcurrent::run(&m_pool, this, &LogWindow::method);
      QPlainTextEdit::keyReleaseEvent(event);
   }
   /// This method must be thread-safe. It is never reentered.
   void method() {
      QThread::sleep(2); // block for two seconds
      g++;
      emit done(g);
   }
   Q_SIGNAL void done(int);
public:
   LogWindow(QWidget * parent = {}) : QPlainTextEdit{parent} {
      appendPlainText("Press and release 'a' a few times.\n");
      m_pool.setMaxThreadCount(1);
      connect(this, &LogWindow::done, this, [this](int val){
         appendPlainText(QString::number(val));
      });
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   LogWindow w;
   w.show();
   return app.exec();
}

#include "main.moc"
