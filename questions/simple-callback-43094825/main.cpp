// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-callback-43094825
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#include <QtConcurrent>
#endif

class MyWindow: public QDialog
{
   Q_OBJECT
   QVBoxLayout m_layout{this};
   QPushButton m_button{"Scan"};
   Q_SIGNAL void ScanFinished();
public:
   MyWindow(QWidget * parent = nullptr) : QDialog(parent) {
      m_layout.addWidget(&m_button);
      connect(&m_button, SIGNAL(clicked(bool)), this, SLOT(Scan()));
      connect(this, SIGNAL(ScanFinished()), this, SLOT(OnScanFinished()));
   }
   Q_SLOT void Scan();
   static void ScanFinishedCallback(void* w);
   Q_SLOT void OnScanFinished();
};

void StartScan(void(*callback)(void*), void* data) {
   // Mockup of the scanning process: invoke the callback after a delay from
   // a worker thread.
   QtConcurrent::run([=]{
      struct Helper : QThread { using QThread::sleep; };
      Helper::sleep(2);
      callback(data);
   });
}

void MyWindow::Scan()
{
   m_button.setEnabled(false);
   StartScan(ScanFinishedCallback, static_cast<void*>(this));
}

void MyWindow::ScanFinishedCallback(void* data)
{
   emit static_cast<MyWindow*>(data)->ScanFinished();
}

void MyWindow::OnScanFinished()
{
   m_button.setEnabled(true);
}

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   MyWindow w;
   w.show();
   return app.exec();
}
#include "main.moc"
