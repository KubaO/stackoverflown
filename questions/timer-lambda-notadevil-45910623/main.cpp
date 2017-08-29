// https://github.com/KubaO/stackoverflown/tree/master/questions/timer-lambda-notadevil-45910623
#include <QtWidgets>

class LogWindow : public QPlainTextEdit {
   Q_OBJECT
   int g = {};
#if __cplusplus < 201103L
   // C++98
   QQueue<int> logQueue;
#endif
   void keyReleaseEvent(QKeyEvent * event) override {
      if (event->key() == Qt::Key_A) {
         g++;
#if __cplusplus >= 201402L
         // C++14
         QTimer::singleShot(2000, this, [this, val=g]{ log(val); });
#elif __cplusplus >= 201103L
         // C++11
         int val = g;
         QTimer::singleShot(2000, this, [=]{ log(val); });
#else
         // C++98
         logQueue.enqueue(g);
         QTimer::singleShot(2000, this, SLOT(log()));
#endif
      }
      QPlainTextEdit::keyReleaseEvent(event);
   }
   void log(int value) {
      appendPlainText(QString::number(value));
   }
   Q_SLOT void log() { // becasue MOC doesn't define __cplusplus :(
#if __cplusplus < 201103L
      // C++98
      log(logQueue.dequeue());
#endif
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   LogWindow w;
   w.appendPlainText("Press and release 'a' a few times.\n");
   w.show();
   return app.exec();
}

#include "main.moc"
