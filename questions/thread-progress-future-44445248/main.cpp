// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-progress-future-44445248
#include <QtWidgets>
#include <QtConcurrent>

class StudentAbsenceTable : public QMainWindow {
   Q_OBJECT
   QStatusBar m_statusBar;
   QProgressBar m_progress;
   QPushButton m_start{"Start Concurrent Task"};
public:
   StudentAbsenceTable() {
      m_statusBar.addPermanentWidget(&m_progress);
      m_progress.setMinimum(0);
      m_progress.setMaximum(0);
      m_progress.setMaximumWidth(150);
      m_progress.hide();
      setStatusBar(&m_statusBar);
      setCentralWidget(&m_start);
      connect(&m_start, &QPushButton::clicked, this, [this]{
         m_start.setEnabled(false);
         QtConcurrent::run(this, &StudentAbsenceTable::worker);
      });
      connect(this, &StudentAbsenceTable::reqStatusMessage,
              this, [this](const QString & msg){
         m_statusBar.showMessage(msg, 3000);
      });
      connect(this, &StudentAbsenceTable::reqFutureActive,
              this, [this](bool active){
         m_progress.setHidden(!active);
         m_start.setEnabled(!active);
      });
   }
private:
   Q_SIGNAL void reqStatusMessage(const QString &);
   Q_SIGNAL void reqFutureActive(bool);
   Q_SLOT void setProgressHidden(bool hidden) { m_progress.setHidden(hidden); }
   bool worker() {
      emit reqFutureActive(true);
      QThread::sleep(3); // pretend to do some work
      emit reqStatusMessage(tr("Oops, something went wrong."));
      emit reqFutureActive(false);
      return false;
   }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   StudentAbsenceTable ui;
   ui.show();
   return app.exec();
}
#include "main.moc"
