// https://github.com/KubaO/stackoverflown/tree/master/questions/comm-commands-32486198
#include <private/qringbuffer_p.h>
#include <QtWidgets>
#include <type_traits>

// See http://stackoverflow.com/a/32317276/1329652
/// A simple point-to-point intra-process pipe. The other endpoint can live in any
/// thread.
class AppPipe : public QIODevice {
   Q_OBJECT
   QRingBuffer m_buf;
   void _a_write(const QByteArray &data) {
      if (!openMode() & QIODevice::ReadOnly) return;  // We must be readable.
      m_buf.append(data);
      emit hasIncoming(data);
      emit readyRead();
   }

  public:
   AppPipe(AppPipe *other, QIODevice::OpenMode mode, QObject *parent = 0)
       : QIODevice(parent) {
      addOther(other);
      open(mode);
   }
   AppPipe(AppPipe *other, QObject *parent = 0) : QIODevice(parent) { addOther(other); }
   void addOther(AppPipe *other) {
      if (other) connect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void removeOther(AppPipe *other) {
      disconnect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void close() Q_DECL_OVERRIDE {
      QIODevice::close();
      m_buf.clear();
   }
   qint64 writeData(const char *data, qint64 maxSize) Q_DECL_OVERRIDE {
      if (!maxSize) return maxSize;
      hasOutgoing(QByteArray(data, maxSize));
      return maxSize;
   }
   qint64 readData(char *data, qint64 maxLength) Q_DECL_OVERRIDE {
      return m_buf.read(data, maxLength);
   }
   qint64 bytesAvailable() const Q_DECL_OVERRIDE {
      return m_buf.size() + QIODevice::bytesAvailable();
   }
   bool canReadLine() const Q_DECL_OVERRIDE {
      return QIODevice::canReadLine() || m_buf.canReadLine();
   }
   bool isSequential() const Q_DECL_OVERRIDE { return true; }
   Q_SIGNAL void hasOutgoing(const QByteArray &);
   Q_SIGNAL void hasIncoming(const QByteArray &);
};

//

template <typename F>
class GuardedSignalTransition : public QSignalTransition {
   F m_guard;

  protected:
   bool eventTest(QEvent *ev) Q_DECL_OVERRIDE {
      return QSignalTransition::eventTest(ev) && m_guard();
   }

  public:
   GuardedSignalTransition(const QObject *sender, const char *signal, F &&guard)
       : QSignalTransition(sender, signal), m_guard(std::move(guard)) {}
   GuardedSignalTransition(const QObject *sender, const char *signal, const F &guard)
       : QSignalTransition(sender, signal), m_guard(guard) {}
};

template <typename F>
static GuardedSignalTransition<F> *addTransition(QState *src, QAbstractState *target,
                                                 const QObject *sender,
                                                 const char *signal, F &&guard) {
   auto t = new GuardedSignalTransition<typename std::decay<F>::type>(
       sender, signal, std::forward<F>(guard));
   t->setTargetState(target);
   src->addTransition(t);
   return t;
}

static bool hasLine(QIODevice *dev, const QByteArray &needle) {
   auto result = false;
   while (dev->canReadLine()) {
      auto line = dev->readLine();
      if (line.contains(needle)) result = true;
   }
   return result;
}

void send(QAbstractState *src, QIODevice *dev, const QByteArray &data) {
   QObject::connect(src, &QState::entered, dev, [dev, data] { dev->write(data); });
}

QTimer *delay(QState *src, int ms, QAbstractState *dst) {
   auto timer = new QTimer(src);
   timer->setSingleShot(true);
   timer->setInterval(ms);
   QObject::connect(src, &QState::entered, timer,
                    static_cast<void (QTimer::*)()>(&QTimer::start));
   QObject::connect(src, &QState::exited, timer, &QTimer::stop);
   src->addTransition(timer, SIGNAL(timeout()), dst);
   return timer;
}

void expect(QState *src, QIODevice *dev, const QByteArray &data, QAbstractState *dst,
            int timeout = 0, QAbstractState *dstTimeout = nullptr) {
   addTransition(src, dst, dev, SIGNAL(readyRead()),
                 [dev, data] { return hasLine(dev, data); });
   if (timeout) delay(src, timeout, dstTimeout);
}

//

class StatefulObject : public QObject {
   Q_OBJECT
   Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
  protected:
   QStateMachine m_mach{this};
   StatefulObject(QObject *parent = 0) : QObject(parent) {}
   void connectSignals() {
      connect(&m_mach, &QStateMachine::runningChanged, this,
              &StatefulObject::runningChanged);
      for (auto state : m_mach.findChildren<QAbstractState *>())
         QObject::connect(state, &QState::entered, this,
                          [this, state] { emit stateChanged(state->objectName()); });
   }

  public:
   Q_SLOT void start() { m_mach.start(); }
   Q_SIGNAL void runningChanged(bool);
   Q_SIGNAL void stateChanged(const QString &);
   bool isRunning() const { return m_mach.isRunning(); }
};

template <class S>
struct NamedState : S {
   NamedState(QState *parent, const char *name) : S(parent) {
      this->setObjectName(QLatin1String(name));
   }
};
typedef NamedState<QState> State;
typedef NamedState<QFinalState> FinalState;

//

class Device : public StatefulObject {
   Q_OBJECT
   AppPipe m_dev{nullptr, QIODevice::ReadWrite, this};
   State s_init{&m_mach, "s_init"}, s_booting{&m_mach, "s_booting"},
       s_firmware{&m_mach, "s_firmware"};
   FinalState s_loaded{&m_mach, "s_loaded"};

  public:
   Device(QObject *parent = 0) : StatefulObject(parent) {
      connectSignals();
      m_mach.setInitialState(&s_init);
      expect(&s_init, &m_dev, "boot", &s_booting);
      delay(&s_booting, 500, &s_firmware);
      send(&s_firmware, &m_dev, "boot successful\n");
      expect(&s_firmware, &m_dev, ":00000001FF", &s_loaded);
      send(&s_loaded, &m_dev, "load successful\n");
   }
   Q_SLOT void stop() { m_mach.stop(); }
   AppPipe &pipe() { return m_dev; }
};

class Programmer : public StatefulObject {
   Q_OBJECT
   AppPipe m_port{nullptr, QIODevice::ReadWrite, this};
   State s_boot{&m_mach, "s_boot"}, s_send{&m_mach, "s_send"};
   FinalState s_ok{&m_mach, "s_ok"}, s_failed{&m_mach, "s_failed"};

  public:
   Programmer(QObject *parent = 0) : StatefulObject(parent) {
      connectSignals();
      m_mach.setInitialState(&s_boot);
      send(&s_boot, &m_port, "boot\n");
      expect(&s_boot, &m_port, "boot successful", &s_send, 1000, &s_failed);
      send(&s_send, &m_port, ":HULLOTHERE\n:00000001FF\n");
      expect(&s_send, &m_port, "load successful", &s_ok, 1000, &s_failed);
   }
   AppPipe &pipe() { return m_port; }
};

static QString formatData(const char *prefix, const char *color, const QByteArray &data) {
   auto text = QString::fromLatin1(data).toHtmlEscaped();
   if (text.endsWith('\n')) text.truncate(text.size() - 1);
   text.replace(QLatin1Char('\n'),
                QString::fromLatin1("<br/>%1 ").arg(QLatin1String(prefix)));
   return QString::fromLatin1("<font color=\"%1\">%2 %3</font><br/>")
       .arg(QLatin1String(color))
       .arg(QLatin1String(prefix))
       .arg(text);
}

int main(int argc, char **argv) {
   using Q = QObject;
   QApplication app{argc, argv};
   Device dev;
   Programmer prog;

   QWidget w;
   QGridLayout grid{&w};
   QTextBrowser comms;
   QPushButton devStart{"Start Device"}, devStop{"Stop Device"},
       progStart{"Start Programmer"};
   QLabel devState, progState;
   grid.addWidget(&comms, 0, 0, 1, 3);
   grid.addWidget(&devState, 1, 0, 1, 2);
   grid.addWidget(&progState, 1, 2);
   grid.addWidget(&devStart, 2, 0);
   grid.addWidget(&devStop, 2, 1);
   grid.addWidget(&progStart, 2, 2);
   devStop.setDisabled(true);
   w.show();

   dev.pipe().addOther(&prog.pipe());
   prog.pipe().addOther(&dev.pipe());
   Q::connect(&prog.pipe(), &AppPipe::hasOutgoing, &comms, [&](const QByteArray &data) {
      comms.append(formatData("&gt;", "blue", data));
   });
   Q::connect(&prog.pipe(), &AppPipe::hasIncoming, &comms, [&](const QByteArray &data) {
      comms.append(formatData("&lt;", "green", data));
   });
   Q::connect(&devStart, &QPushButton::clicked, &dev, &Device::start);
   Q::connect(&devStop, &QPushButton::clicked, &dev, &Device::stop);
   Q::connect(&dev, &Device::runningChanged, &devStart, &QPushButton::setDisabled);
   Q::connect(&dev, &Device::runningChanged, &devStop, &QPushButton::setEnabled);
   Q::connect(&dev, &Device::stateChanged, &devState, &QLabel::setText);
   Q::connect(&progStart, &QPushButton::clicked, &prog, &Programmer::start);
   Q::connect(&prog, &Programmer::runningChanged, &progStart, &QPushButton::setDisabled);
   Q::connect(&prog, &Programmer::stateChanged, &progState, &QLabel::setText);
   return app.exec();
}

#include "main.moc"
