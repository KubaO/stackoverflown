#include <QApplication>
#include <QtWidgets>
#include <QTimer>

class Widget : public QWidget
{
   Q_OBJECT
public:
   explicit Widget(QWidget *parent = 0);
private slots:
   void setTimer();
   void displayAdvice();
   void cancelTimer();
   void addAdvice();
   void processDone(int);
private:
   QLabel m_timerLbl;
   QLineEdit m_timerEdt;
   QTextEdit m_adviceEdt;
   QPushButton m_okBtn;
   QPushButton m_cancelBtn;
   QTimer m_timer;
   QProcess m_process;
   void setupGui();
};

Widget::Widget(QWidget *parent) :
   QWidget(parent)
{
   setupGui();
}

void Widget::setupGui()
{
   m_timerLbl.setText("Timer set interval in seconds");
   m_adviceEdt.setReadOnly(true);
   m_okBtn.setText("OK");
   m_cancelBtn.setText("Cancel");
   m_cancelBtn.setEnabled(false);
   QVBoxLayout* layout = new QVBoxLayout(this);
   layout->addWidget(&m_timerLbl);
   layout->addWidget(&m_timerEdt);
   layout->addWidget(&m_okBtn);
   layout->addWidget(&m_adviceEdt);
   layout->addWidget(&m_cancelBtn);
   setWindowTitle("Advice");

   connect(&m_okBtn, SIGNAL(clicked()), SLOT(setTimer()));
   connect(&m_cancelBtn, SIGNAL(clicked()), SLOT(cancelTimer()));
   connect(&m_timer, SIGNAL(timeout()), SLOT(displayAdvice()));
   connect(&m_timer, SIGNAL(timeout()), SLOT(cancelTimer()));

   connect(&m_process, SIGNAL(readyReadStandardOutput()), SLOT(addAdvice()));
   connect(&m_process, SIGNAL(finished(int)), SLOT(processDone(int)));
}

void Widget::setTimer()
{
   QString const setting = m_timerEdt.text();
   bool ok;
   int set = setting.toInt(&ok,10) * 1000;
   m_timer.setInterval(set);
   m_timer.start();
   m_timerEdt.setEnabled(false);
   m_okBtn.setEnabled(false);
   m_cancelBtn.setEnabled(true);
}

void Widget::cancelTimer()
{
   m_timer.stop();
   m_adviceEdt.clear();
   m_timerEdt.clear();
   m_okBtn.setEnabled(true);
   m_timerEdt.setEnabled(true);
   m_cancelBtn.setEnabled(false);
}

void Widget::displayAdvice()
{
   m_process.start("bash", QStringList() << "-c" << "echo 'Hello!'");
#if 0
   m_process.start(QDir::homePath() +
                   "/Desktop/47039949 COS3711 Assignment 2/Question 4/Ass2Q4Part1-build-desktop/debug/Ass2Q4Part1.exe");
#endif
}

void Widget::addAdvice()
{
   QByteArray const data = m_process.readAllStandardOutput();
   m_adviceEdt.setPlainText(QString::fromLocal8Bit(data));
}

void Widget::processDone(int)
{
   m_process.close();
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Widget w;
   w.show();
   return a.exec();
}

#include "main.moc"
