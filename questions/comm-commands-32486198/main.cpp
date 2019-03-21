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
   auto *tr = new GuardedSignalTransition<typename std::decay<F>::type>(
       sender, signal, std::forward<F>(guard));
   tr->setTargetState(target);
   src->addTransition(tr);
   return tr;
}

static bool hasLine(QIODevice *dev, const QByteArray &needle) {
   auto result = false;
   while (dev->canReadLine()) {
      auto line = dev->readLine();
      if (line.contains(needle)) result = true;
   }
   return result;
}

class StateBuilder {
   QStateMachine *m_machine;
   QIODevice *m_device;
   QAbstractState *m_parentState = {}, *m_prevState = {}, *m_state = {};
   QMap<QString, QAbstractTransition *> m_destinations;
   QAbstractState *point(QAbstractTransition *tr, const QVariant &dst) {
      Q_ASSERT(tr);
      if (dst.canConvert<QAbstractState *>()) {
         auto *d = qvariant_cast<QAbstractState *>(dst);
         if (!d)
            m_destinations.insertMulti({}, tr);
         else
            tr->setTargetState(d);
         return d;
      }
      m_destinations.insertMulti(dst.toString(), tr);
      return {};
   }
   QState *parent() const {
      auto *p = qobject_cast<QState *>(m_parentState);
      return p ? p : m_machine;
   }
   QAbstractState *src() const {
      Q_ASSERT(m_state);
      return m_state;
   }
   QState *srcState() {
      Q_ASSERT(m_state);
      return qobject_cast<QState *>(m_state);
   }
   template <typename T>
   T *childState(const QString &name) const {
      return parent() ? parent()->findChild<T *>(name, Qt::FindDirectChildrenOnly)
                      : nullptr;
   }

  public:
   struct TargetState : public QVariant {
      TargetState() = default;
      TargetState(const QString &s) : QVariant(s) {}
      TargetState(const char *s) : QVariant(QLatin1String(s)) {}
      TargetState(QAbstractState *s) : QVariant(QVariant::fromValue(s)) {}
      explicit operator bool() const {
         return !isNull() && (canConvert<QString>() || value<QAbstractState *>());
      }
   };

   explicit StateBuilder(QStateMachine *machine) : m_machine(machine) {}
   StateBuilder &next(QAbstractState *state = {}) {
      m_prevState = m_state;
      m_state = state;
      return *this;
   }
   template <typename T = QState, typename = typename std::enable_if<
                                      std::is_base_of<QAbstractState, T>::value>::type>
   StateBuilder &next(const QString &name) {
      auto *state = childState<T>(name);
      if (!state) {
         state = new T(state);
         state->setObjectName(name);
         auto setDestinations = [this](const QString &name, QAbstractState *state) {
            for (auto *tr : m_destinations.values(name)) tr->setTargetState(state);
            m_destinations.remove(name);
         };
         setDestinations(name, state);
         if (!name.isEmpty()) setDestinations({}, state);
      }
      return next(state);
   }

   void setDevice(QIODevice *dev) { m_device = dev; }
   QTimer *delay(int ms, const TargetState &dst = {}) {
      auto timer = new QTimer(src());
      timer->setSingleShot(true);
      timer->setInterval(ms);
      QObject::connect(src(), &QState::entered, timer, QOverload<>::of(&QTimer::start));
      QObject::connect(src(), &QState::exited, timer, &QTimer::stop);
      auto *tr = new QSignalTransition(timer, &QTimer::timeout);
      srcState()->addTransition(tr);
      point(tr, dst);
      return timer;
   }
   void send(QIODevice *dev, const QByteArray &data) {
      QObject::connect(src(), &QState::entered, dev, [dev, data] { dev->write(data); });
   }
   void expect(QIODevice *dev, const QByteArray &data, const TargetState &dst = {},
               int timeout = 0, const TargetState &dstTimeout = {}) {
      point(addTransition(srcState(), {}, dev, SIGNAL(readyRead()),
                          [dev, data] { return hasLine(dev, data); }),
            dst);
      if (timeout) delay(timeout, dstTimeout);
   }
   void send(const QByteArray &data) { send(m_device, data); }
   void expect(const QByteArray &data, const TargetState &dst = {}, int timeout = 0,
               const TargetState &dstTimeout = {}) {
      expect(m_device, data, dst, timeout, dstTimeout);
   }
   void final(const QString &name = {}) {}
};

//

class StatefulObject : public QObject {
   Q_OBJECT
   Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
  protected:
   QStateMachine m_mach{this};
   StateBuilder m_builder{&m_mach};
   StateBuilder &sb() { return m_builder; }
   StateBuilder &on(const QString &name) { return m_builder.next(name); }
   StateBuilder &on(QAbstractState *state) { return m_builder.next(state); }
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
#if 0
   State s_init{&m_mach, "s_init"}, s_booting{&m_mach, "s_booting"},
       s_firmware{&m_mach, "s_firmware"};
   FinalState s_loaded{&m_mach, "s_loaded"};
#endif

  public:
   Device(QObject *parent = 0) : StatefulObject(parent) {
      connectSignals();
      m_mach.setInitialState(&s_init);
      m_builder.setDevice(&m_dev);
      on("s_init").expect("boot");
      on("s_booting").delay(500);
      on("s_firmware").send("boot successful\n");
      sb().expect(":00000001FF");
      on("s_loaded").send("load successful\n");
      sb().final("s_loaded");
   }
   Q_SLOT void stop() { m_mach.stop(); }
   AppPipe &pipe() { return m_dev; }
};

class Programmer : public StatefulObject {
   Q_OBJECT
   AppPipe m_port{nullptr, QIODevice::ReadWrite, this};
#if 0
   State s_boot{&m_mach, "s_boot"}, s_send{&m_mach, "s_send"};
   FinalState s_ok{&m_mach, "s_ok"}, s_failed{&m_mach, "s_failed"};
#endif

  public:
   Programmer(QObject *parent = 0) : StatefulObject(parent) {
      connectSignals();
      m_mach.setInitialState(&s_boot);
      m_builder.setDevice(&m_port);
      on("s_boot").send("boot\n");
      sb().expect("boot successful", "s_send", 1000, "s_failed");
      on("s_send").send(":HULLOTHERE\"n:00000001FF\n");
      sb().expect("load successful", "s_ok", 1000, "s_failed");
      sb().final("s_ok");
      sb().final("s_failed");
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
