// https://github.com/KubaO/stackoverflown/tree/master/questions/threadwork-simple-40865259
#include <QtWidgets>
#include <QtConcurrent>

struct ApplicationData {};

struct OptimizationAlgorithm {
   void timeConsumingMethod(QSharedPointer<ApplicationData>) {
      QThread::sleep(3);
   }
};

class Controller : public QObject {
   Q_OBJECT
   QSharedPointer<ApplicationData> m_data{new ApplicationData};
   OptimizationAlgorithm m_algorithm;
public:
   Q_SLOT void run() {
      QtConcurrent::run([this]{
         emit busy();
         m_algorithm.timeConsumingMethod(m_data);
         emit finished();
      });
   }
   Q_SIGNAL void busy();
   Q_SIGNAL void finished();
};

class Registration : public QWidget {
   Q_OBJECT
   QVBoxLayout m_layout{this};
   QLabel m_status{"Idle"};
   QPushButton m_run{"Run"};
public:
   Registration() {
      m_layout.addWidget(&m_status);
      m_layout.addWidget(&m_run);
      connect(&m_run, &QPushButton::clicked, this, &Registration::reqRun);
   }
   Q_SIGNAL void reqRun();
   Q_SLOT void onBusy() { m_status.setText("Running"); }
   Q_SLOT void onFinished() { m_status.setText("Idle"); }
};

void setup(Registration *reg, Controller *ctl) {
   using Q = QObject;
   Q::connect(reg, &Registration::reqRun, ctl, &Controller::run);
   Q::connect(ctl, &Controller::busy, reg, &Registration::onBusy);
   Q::connect(ctl, &Controller::finished, reg, &Registration::onFinished);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Controller ctl;
   Registration reg;
   setup(&reg, &ctl);
   reg.show();
   return app.exec();
}
#include "main.moc"
