#include <QApplication>
#include <QGridLayout>
#include <QSpinBox>
#include <QTimer>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget w;
   QGridLayout * layout = new QGridLayout(&w);
   QSpinBox * spin = new QSpinBox;
   layout->addWidget(spin);
   layout->addWidget(new QSpinBox);
   QTimer timer;
   timer.setInterval(1000);
   timer.start();
   QObject::connect(&timer, &QTimer::timeout, [=]{
      //spin->setValue(spin->value()+1);
      spin->stepUp();
   });
   w.show();
   return a.exec();
}
