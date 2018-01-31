// https://github.com/KubaO/stackoverflown/tree/master/questions/static-signal-48540601
#include <QtCore>

class Main : public QObject {
   Q_OBJECT
   static Main * m_instance;
public:
   Main(QObject * parent = {}) : QObject(parent) {
      Q_ASSERT(! hasInstance());
      m_instance = this;
   }
   static Main * instance() {
      if (! m_instance) m_instance = new Main;
      return m_instance;
   }
   static bool hasInstance() { return m_instance; }
   Q_SIGNAL void theSignal();
};

Main * Main::m_instance;

void callback() {
   Q_ASSERT(Main::hasInstance());
   // If the instance didn't exist here, nothing can receive the signal.
   Main::instance()->theSignal();
}

int main()
{
   int slotCalls = {};
   Main object;
   QObject::connect(&object, &Main::theSignal, [&]{ slotCalls ++; });
   Q_ASSERT(slotCalls == 0);
   callback();
   Q_ASSERT(slotCalls == 1);
}

#include "main.moc"
