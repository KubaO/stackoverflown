#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   QWidget w;
   QHBoxLayout layout{&w};
   QPushButton button1{"Default"};
   button1.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
   layout.addWidget(&button1);
   QPushButton button2{"Styled"};
   button2.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   button2.setStyleSheet(
            "* { border: 2px solid #8f8f91; border-radius: 12px; background-color: #d02020; }"
            "*:pressed { background-color: #f6f7fa; }");
   layout.addWidget(&button2);
   auto pal = w.palette();
   pal.setBrush(QPalette::Background, Qt::darkBlue);
   w.setPalette(pal);
   w.show();
   return a.exec();
}
