#include <QApplication>
#include <QWidget>
#include <QPushButton>

class Second : public QWidget
{
   QPushButton bout1, bout2, bout3;
public:
   Second();
};

Second::Second() :
   bout1("button1", this),
   bout2("button2", this),
   bout3("button3", this)
{
   setFixedSize(700, 150);
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Second sec;
   sec.show();
   return a.exec();
}
