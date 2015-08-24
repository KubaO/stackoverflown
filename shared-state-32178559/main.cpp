#include <QtWidgets>
#include <random>

typedef QHash<QString, double> Map;

class Computer : public QObject {
   Q_OBJECT
   std::default_random_engine m_eng;
   Map m_map;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      auto n = std::uniform_int_distribution<int>(0, m_map.size()-1)(m_eng);
      auto it = m_map.begin();
      std::advance(it, n);
      auto val = std::normal_distribution<double>()(m_eng);
      *it = val;
      emit valueChanged(it.key(), it.value());
      m_timer.start(std::uniform_int_distribution<int>(100, 1000)(m_eng), this);
   }
public:
   Computer(const Map & map = Map(), QObject * parent = 0) :
      QObject(parent),
      m_map(map) {
      m_timer.start(0, this);
   }
   Q_SIGNAL void valueChanged(const QString & key, double val);
   Q_SLOT void stop() { m_timer.stop(); }
   Map data() const { return m_map; }
};

class UI : public QWidget {
   Q_OBJECT
   QHash<QString, int> m_row;
   QGridLayout m_layout { this };
   QStandardItemModel m_model;
   QTableView m_view;
public:
   UI(const Map & map = Map(), QWidget * parent = 0) :
      QWidget(parent),
      m_model(map.size(), 2, this)
   {
      m_layout.addWidget(&m_view, 0, 0);
      int row = 0;
      for (auto it = map.begin(); it != map.end(); ++it) {
         m_model.setItem(row, 0, new QStandardItem(it.key()));
         m_model.setItem(row, 1, new QStandardItem(QString::number(it.value())));
         m_row[it.key()] = row++;
      }
      m_view.setModel(&m_model);
   }
   Q_SLOT void newValue(const QString & key, double val) {
      m_model.item(m_row[key], 1)->setText(QString::number(val));
   }
};

struct Thread : public QThread { ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QHash<QString, double> init;
   init.insert("Foo", 1);
   init.insert("Bar", 2);
   init.insert("Baz", 3);
   Computer computer(init);
   Thread thread;
   UI ui(computer.data());
   QObject::connect(&computer, &Computer::valueChanged, &ui, &UI::newValue);
   // Prevent the bogus "QBasicTimer::stop: Failed." warnings.
   QObject::connect(&thread, &QThread::finished, &computer, &Computer::stop);
   computer.moveToThread(&thread);
   thread.start();
   ui.show();
   return a.exec();
}

#include "main.moc"
