#if 1
#include <QtWidgets>

class MyProcessor : public QObject {
  Q_OBJECT
  Q_PROPERTY(int value READ value NOTIFY valueChanged)
  QBasicTimer m_timer;
  int m_value;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_timer.timerId()) return;
    m_value = m_value == 42 ? 0 : 42;
    emit valueChanged(m_value);
  }
public:
  MyProcessor(QObject * parent = 0) : QObject(parent) { m_timer.start(1000, this); }
  Q_SIGNAL void valueChanged(int);
  int value() const { return m_value; }
};

class Gui : public QObject {
  Q_OBJECT
  QLabel m_label;
public:
  Gui(QObject * parent = 0) : QObject(parent) {
    m_label.setMinimumWidth(qApp->fontMetrics().averageCharWidth() * 30);
    m_label.show();
  }
  Q_SLOT void newValue(int value) {
    m_label.setText(value == 42 ? "OK, correct value" : "Oops, wrong value");
  }
};

// We must fix the broken-by-design QThread
struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
  QApplication app(argc, argv);
  Gui gui;
  MyProcessor proc;
  Thread procThread;
  proc.moveToThread(&procThread);
  procThread.start();
  QObject::connect(&proc, &MyProcessor::valueChanged, &gui, &Gui::newValue);
  return app.exec();
}
#include "main.moc"
#endif

#if 0
#include <QtWidgets>
// USE ONLY IF MyProcessor PROPERTIES HAVE NO CHANGE NOTIFIERS

class MyProcessor : public QObject {
  Q_OBJECT
  Q_PROPERTY(int value READ value)
  QBasicTimer m_timer;
  int m_value;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_timer.timerId()) return;
    m_value = m_value == 42 ? 0 : 42;
  }
public:
  MyProcessor(QObject * parent = 0) : QObject(parent) { m_timer.start(1000, this); }
  int value() const { return m_value; }
};

// See http://stackoverflow.com/a/21653558/1329652 for discussion of postTo
template <typename F>
void postTo(QObject * obj, F && fun) {
  if (!obj) return;
  if (obj->thread() != QThread::currentThread()) {
    QObject signalSource;
    QObject::connect(&signalSource, &QObject::destroyed, obj, std::move(fun));
  } else
    fun();
}

class Gui : public QObject {
  Q_OBJECT
  QBasicTimer m_poll;
  QLabel m_label;
  QPointer<MyProcessor> m_proc;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_poll.timerId()) return;
    postTo(m_proc, [=]{
      // runs in target's thread
      auto value = m_proc->value();
      postTo(this, [=]{ newValue(value); /* runs in our thread */ });
    });
  }
public:
  Gui(MyProcessor * proc, QObject * parent = 0) : QObject(parent), m_proc(proc) {
    m_label.setMinimumWidth(qApp->fontMetrics().averageCharWidth() * 30);
    m_label.show();
    m_poll.start(900, this);
  }
  Q_SLOT void newValue(int value) {
    m_label.setText(value == 42 ? "OK, correct value" : "Oops, wrong value");
  }
};

// We must fix the broken-by-design QThread
struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
  QApplication app(argc, argv);
  MyProcessor proc;
  Thread procThread;
  proc.moveToThread(&procThread);
  procThread.start();
  Gui gui(&proc);
  return app.exec();
}
#include "main.moc"
#endif

#if 0
// DO NOT USE THIS CODE IN PRODUCTION!
// THIS IS A HORRIBLE USABILITY BUG
// ONLY USE FOR DEBUGGING!!
#include <QtWidgets>

class MyProcessor : public QObject {
  Q_OBJECT
  Q_PROPERTY(int value READ value)
  QBasicTimer m_timer;
  int m_value;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_timer.timerId()) return;
    m_value = m_value == 42 ? 0 : 42;
  }
public:
  MyProcessor(QObject * parent = 0) : QObject(parent) { m_timer.start(1000, this); }
  Q_SLOT int value() const { return m_value; }
};

class Gui : public QObject {
  Q_OBJECT
  QBasicTimer m_poll;
  QLabel m_label;
  QPointer<MyProcessor> m_proc;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_poll.timerId()) return;
    newValue(getValue());
  }
public:
  Gui(MyProcessor * proc, QObject * parent = 0) : QObject(parent), m_proc(proc) {
    m_label.setMinimumWidth(qApp->fontMetrics().averageCharWidth() * 30);
    m_label.show();
    m_poll.start(900, this);
  }
  Q_SIGNAL int getValue();
  Q_SLOT void newValue(int value) {
    m_label.setText(value == 42 ? "OK, correct value" : "Oops, wrong value");
  }
};

// We must fix the broken-by-design QThread
struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
  QApplication app(argc, argv);
  MyProcessor proc;
  Thread procThread;
  proc.moveToThread(&procThread);
  procThread.start();
  Gui gui(&proc);
  QObject::connect(&gui, &Gui::getValue, &proc, &MyProcessor::value, Qt::BlockingQueuedConnection);
  return app.exec();
}
#include "main.moc"
#endif
