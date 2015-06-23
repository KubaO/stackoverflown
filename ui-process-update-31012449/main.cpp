#if 1

#include <QApplication>
#include <QGridLayout>
#include <QProcess>
#include <QLabel>
#include <QTimer>
#include <QTextStream>
#include <QRegExp>
#include <cstdio>

// QT 5, C++11
int main(int argc, char *argv[])
{
   if (argc > 1) {
      QCoreApplication app(argc, argv);
      // output 3 random values per line at ~20Hz
      QTextStream out(stdout);
      QTimer timer;
      timer.start(50);
      QObject::connect(&timer, &QTimer::timeout, [&out]{
         out << qrand() << " " << qrand() << " " << qrand() << endl;
      });
      return app.exec();
   }
   QApplication app(argc, argv);
   QWidget w;
   QGridLayout layout(&w);
   QLabel l1, l2, l3;
   layout.addWidget(&l1, 0, 0);
   layout.addWidget(&l2, 0, 1);
   layout.addWidget(&l3, 0, 2);
   QProcess process;
   process.start(QCoreApplication::applicationFilePath(), QStringList("foo"));
   QObject::connect(&process, &QProcess::readyRead, [&]{
      static QRegExp sep("\\W+");
      while (process.canReadLine()) {
         QStringList data = QString::fromLocal8Bit(process.readLine()).split(sep, QString::SkipEmptyParts);
         if (data.length() != 3) continue;
         l1.setText(data.at(0));
         l2.setText(data.at(1));
         l3.setText(data.at(2));
      }
   });
   app.setQuitOnLastWindowClosed(false);
   process.connect(&app, SIGNAL(lastWindowClosed()), SLOT(terminate()));
   app.connect(&process, SIGNAL(finished(int)), SLOT(quit()));
   w.show();
   return app.exec();
}

#endif

#if 0

#include <QApplication>
#include <QGridLayout>
#include <QProcess>
#include <QLabel>
#include <QTimer>
#include <QTextStream>
#include <QRegExp>
#include <QPointer>
#include <cstdio>

// QT 4, C++98
class Emulator : public QObject {
   Q_OBJECT
   QTextStream m_out;
   QTimer m_timer;
   Q_SLOT void on_timeout() {
      m_out << qrand() << " " << qrand() << " " << qrand() << endl;
   }
public:
   Emulator() : m_out(stdout) {
      m_timer.start(50);
      connect(&m_timer, SIGNAL(timeout()), SLOT(on_timeout()));
   }
};

class Widget : public QWidget {
   Q_OBJECT
   QGridLayout m_layout;
   QLabel m_l1, m_l2, m_l3;
   QPointer<QProcess> m_process;
   Q_SLOT void on_readyRead() {
      static QRegExp sep("\\W+");
      while (m_process->canReadLine()) {
         QStringList data = QString::fromLocal8Bit(m_process->readLine()).split(sep, QString::SkipEmptyParts);
         if (data.length() != 3) continue;
         m_l1.setText(data.at(0));
         m_l2.setText(data.at(1));
         m_l3.setText(data.at(2));
      }
   }
public:
   Widget(QProcess * process) : m_layout(this), m_process(process) {
      m_layout.addWidget(&m_l1, 0, 0);
      m_layout.addWidget(&m_l2, 0, 1);
      m_layout.addWidget(&m_l3, 0, 2);
      connect(m_process, SIGNAL(readyRead()), SLOT(on_readyRead()));
   }
};

int main(int argc, char *argv[])
{
   if (argc > 1) {
      // output 3 random values per line at ~20Hz
      QCoreApplication app(argc, argv);
      Emulator emulator;
      return app.exec();
   }
   QApplication app(argc, argv);
   QProcess process;
   Widget w(&process);
   process.start(QCoreApplication::applicationFilePath(), QStringList("foo"));
   app.setQuitOnLastWindowClosed(false);
   process.connect(&app, SIGNAL(lastWindowClosed()), SLOT(terminate()));
   app.connect(&process, SIGNAL(finished(int)), SLOT(quit()));
   w.show();
   return app.exec();
}
#include "main.moc"

#endif
