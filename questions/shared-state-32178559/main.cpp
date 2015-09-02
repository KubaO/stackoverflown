#include <QtWidgets>
#include <random>

class AbstractComputer : public QObject {
   Q_OBJECT
   Q_PROPERTY (Mode mode READ mode WRITE setMode NOTIFY modeChanged)
   Q_PROPERTY (double simTime READ simTime WRITE setSimTime
               RESET resetSimTime NOTIFY simTimeChanged)
public:
   enum Mode { Stop, Step, RealTime, Fast };
protected:
   typedef double Time; ///< units of seconds

   /// Performs one computation step and returns the amount of time the simulation has
   /// been advanced by. The computation updates one or more parameters in the map, but
   /// doesn't signal the updates. The changed parameters are kept in set.
   /// This method can change m_mode to Stop to direct the calling code to stop/pause
   /// the simulation.
   virtual Time compute() = 0;

   /// Notifies of accumulated changes and clears the update set.
   virtual void notify() = 0 ;
private:
   Mode m_mode, m_prevMode;
   QBasicTimer m_timer;
   QElapsedTimer m_timeBase;
   qint64 m_lastNotification; ///< Last m_timeBase at which notification was issued.
   Time m_notifyPeriod; ///< Real time period to issue data change notifications at.
   Time m_modeSimTime;  ///< Simulation time accumulated in current mode.
   Time m_simTime;      ///< Total simulation time.

   /// Computes a chunk of work that amounts to m_notifyPeriod in simulated time
   void computeChunk() {
      Time t = 0;
      do
         t += compute();
      while (m_mode != Stop && t < m_notifyPeriod);
      m_modeSimTime += t;
      m_simTime += t;
   }

   /// Runs computations according to the selected mode. In RealTime and Fast modes,
   /// the notifications are issued at least every m_notifyPeriod.
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      const Time startSimTime = m_simTime;
      const Mode startMode = m_mode;
      switch (m_mode) {
      case Step:
         m_simTime += compute();
         m_timer.stop();
         m_mode = Stop;
         break;
      case Stop:
         m_timer.stop();
         break;
      case RealTime:
         if (m_prevMode != RealTime) {
            m_modeSimTime = 0.0;
            m_timeBase.start();
         }
         computeChunk();
         if (m_mode == RealTime) {
            int ahead = round(m_modeSimTime * 1000.0 - m_timeBase.elapsed());
            if (ahead < 0) ahead = 0;
            m_timer.start(ahead, Qt::PreciseTimer, this);
         }
         break;
      case Fast:
         if (m_prevMode != Fast) {
            m_timeBase.start();
            m_lastNotification = 0;
         }
         do
            computeChunk();
         while (m_mode == Fast
                && ((m_timeBase.elapsed() - m_lastNotification) < m_notifyPeriod*1000.0));
         m_lastNotification = m_timeBase.elapsed();
         break;
      }
      notify();
      if (startSimTime != m_simTime) emit simTimeChanged(m_simTime);
      if (m_prevMode != m_mode || startMode != m_mode) emit modeChanged(m_mode);
      m_prevMode = m_mode;
   }
public:
   AbstractComputer(QObject * parent = 0) :
      QObject(parent), m_mode(Stop), m_prevMode(Stop), m_notifyPeriod(0.02) /* 50 Hz */,
      m_simTime(0.0)
   {}
   Q_SIGNAL void modeChanged(AbstractComputer::Mode mode); // fully qualified type is required by moc
   Q_SIGNAL void simTimeChanged(double);
   Q_SLOT void setMode(AbstractComputer::Mode mode) { // fully qualified type is required by moc
      if (m_mode == mode) return;
      m_mode = mode;
      if (m_mode != Stop) m_timer.start(0, this); else m_timer.stop();
   }
   Q_SLOT void stop() { setMode(Stop); }
   Mode mode() const { return m_mode; }
   double simTime() const { return m_simTime; }
   void setSimTime(double t) { if (m_simTime != t) { m_simTime = t; emit simTimeChanged(t); } }
   void resetSimTime() { setSimTime(0.0); }
};
Q_DECLARE_METATYPE(AbstractComputer::Mode)

class Computer : public AbstractComputer {
   Q_OBJECT
public:
   typedef QHash<QString, double> Map;
private:
   typedef QSet<QString> Set;
   std::default_random_engine m_eng;
   Map m_data;
   Set m_updates;

   Time compute() Q_DECL_OVERRIDE {
      // Update one randomly selected parameter.
      auto n = std::uniform_int_distribution<int>(0, m_data.size()-1)(m_eng);
      auto it = m_data.begin();
      std::advance(it, n);
      auto val = std::normal_distribution<double>()(m_eng);
      *it = val;
      m_updates.insert(it.key());
      float tau = std::uniform_real_distribution<float>(0.001, 0.1)(m_eng);
      // Pretend that we run ~4x faster than real time
      QThread::usleep(tau*1E6/4.0);
      return tau;
   }
   void notify() Q_DECL_OVERRIDE {
      for (auto param : m_updates)
         emit valueChanged(param, m_data[param]);
      m_updates.clear();
   }
public:
   Computer(const Map & data, QObject * parent = 0) :
      AbstractComputer(parent), m_data(data) {}
   Map data() const { return m_data; }
   Q_SIGNAL void valueChanged(const QString & key, double val);
};

class UI : public QWidget {
   Q_OBJECT
   QHash<QString, int> m_row;
   QStandardItemModel m_model;
   QFormLayout m_layout { this };
   QTableView m_view;
   QComboBox m_mode;
public:
   UI(const Computer * computer, QWidget * parent = 0) :
      QWidget(parent),
      m_model(computer->data().size() + 1, 2, this)
   {
      auto data = computer->data();
      m_mode.addItem("Stop", Computer::Stop);
      m_mode.addItem("Step", Computer::Step);
      m_mode.addItem("Real Time", Computer::RealTime);
      m_mode.addItem("Fast", Computer::Fast);
      m_mode.setFocusPolicy(Qt::StrongFocus);
      m_view.setFocusPolicy(Qt::NoFocus);
      m_layout.addRow(&m_view);
      m_layout.addRow("Sim Mode", &m_mode);
      m_model.setItem(0, 0, new QStandardItem("Sim Time [s]"));
      m_model.setItem(0, 1, new QStandardItem);
      int row = 1;
      for (auto it = data.begin(); it != data.end(); ++it) {
         m_model.setItem(row, 0, new QStandardItem(it.key()));
         m_model.setItem(row, 1, new QStandardItem(QString::number(it.value())));
         m_row[it.key()] = row++;
      }
      newMode(computer->mode());
      newSimTime(computer->simTime());
      m_view.setModel(&m_model);
      connect(&m_mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
              [this](int i){
         emit modeChanged((AbstractComputer::Mode)m_mode.itemData(i).toInt());
      });
   }
   Q_SIGNAL void modeChanged(AbstractComputer::Mode);
   Q_SLOT void newValue(const QString & key, double val) {
      m_model.item(m_row[key], 1)->setText(QString::number(val));
   }
   Q_SLOT void newSimTime(double t) {
      m_model.item(0, 1)->setText(QString::number(t));
   }
   Q_SLOT void newMode(AbstractComputer::Mode mode) {
      m_mode.setCurrentIndex(m_mode.findData(mode));
   }
};

struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   qRegisterMetaType<AbstractComputer::Mode>();
   Computer::Map init;
   init.insert("Foo", 1);
   init.insert("Bar", 2);
   init.insert("Baz", 3);
   Computer computer(init);
   QScopedPointer<Thread> thread;
   UI ui(&computer);
   QObject::connect(&computer, &Computer::valueChanged, &ui, &UI::newValue);
   QObject::connect(&computer, &Computer::simTimeChanged, &ui, &UI::newSimTime);
   QObject::connect(&computer, &Computer::modeChanged, &ui, &UI::newMode);
   QObject::connect(&ui, &UI::modeChanged, &computer, &Computer::setMode);
   int threadCount = Thread::idealThreadCount();
   if (threadCount == -1 || threadCount > 1) { // Assume a multicore machine
      thread.reset(new Thread);
      computer.moveToThread(thread.data());
      thread->start();
      // Prevent the bogus "QBasicTimer::stop: Failed." warnings.
      QObject::connect(thread.data(), &QThread::finished, &computer, &Computer::stop);
   }
   ui.show();
   return a.exec();
}

#include "main.moc"
