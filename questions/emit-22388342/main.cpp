#include <QApplication>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QTimer>
#include <QPushButton>

class Counter : public QWidget
{
   Q_OBJECT
public:
   explicit Counter(QWidget *parent = 0) : QWidget(parent) {
      QTimer *timer = new QTimer(this);
      connect(timer, SIGNAL(timeout()), SLOT(run()));
      timer->start(1000);

      QVBoxLayout *layout = new QVBoxLayout(this);
      QPushButton *OK = new QPushButton("OK");
      connect(OK, SIGNAL(clicked()), SLOT(OKalarmLimits()));
      layout->addWidget(OK);
   }
   Q_SIGNAL void textChanged(const QString &text);
   Q_SLOT void run() { Q_EMIT textChanged("Run - Counter"); }
   Q_SLOT void OKalarmLimits() { Q_EMIT textChanged("Button Clicked"); }
};

class MainWindow : public QWidget {
   Q_OBJECT
   QPlainTextEdit *box;
public:
   explicit MainWindow(QWidget *parent = 0) : QWidget(parent) {
      QVBoxLayout * layout = new QVBoxLayout(this);

      box = new QPlainTextEdit();
      box->setMaximumHeight(400);
      box->setMinimumWidth(400);
      layout->addWidget(box);

#if 0
      Counter * counter = new Counter();
      connect(counter, SIGNAL(textChanged(QString)), SLOT(updateWidgets(QString)));
      QTabWidget *tabWidget = new QTabWidget;
      tabWidget->addTab(counter, tr("Counter"));
      layout->addWidget(tabWidget);
#else
      QTabWidget *tabWidget = new QTabWidget;
      tabWidget->addTab(new Counter(), tr("Counter"));
      layout->addWidget(tabWidget);
#endif

      QTimer *timer = new QTimer(this);
      connect(timer, SIGNAL(timeout()), SLOT(run()));
      timer->start(1000);
   }
   Q_SLOT void updateWidgets(const QString &t) { box->appendPlainText(t); }
   Q_SLOT void run() { box->appendPlainText("Run - Window"); }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MainWindow s;
   Counter m;
   s.show();
   s.connect(&m, SIGNAL(textChanged(QString)), SLOT(updateWidgets(QString)));
   return a.exec();
}

#include "main.moc"
