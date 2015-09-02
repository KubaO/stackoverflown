#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QLabel label1("Hello, World!");
   label1.show();
   label1.setMinimumSize(200, 100);
   app.exec();

   QTimer timer;
   int counter = 0;
   QObject::connect(&timer, &QTimer::timeout, [&counter]{ counter ++; });
   timer.start(100);
   QMessageBox::information(nullptr, "Done",
                            QString("We're Done with \"%1\"").arg(label1.text()));
   QMessageBox::information(nullptr, "Really Done",
                            QString("Timer has ticked %1 times").arg(counter));
   return 0;
}
