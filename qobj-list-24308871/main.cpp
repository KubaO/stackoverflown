#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <list>
#include <memory>
#include <boost/ptr_container/ptr_list.hpp>

int main(int argc, char *argv[])
{
   int i = 0;
   QApplication a(argc, argv);
   QWidget w;
   QHBoxLayout layout(&w);
   // C++11 - emplace
   std::list<QLabel> labels1;
   for (; i<5; ++i) {
      labels1.emplace_back(QString::number(i));
      layout.addWidget(&labels1.back());
   }
   // C++11 - std::unique_ptr
   std::list<std::unique_ptr<QLabel>> labels2;
   for (; i<10; ++i) {
      if (i%1)
         // with emplace
         labels2.emplace_back(new QLabel(QString::number(i)));
      else
         // with push_back
         labels2.push_back(std::unique_ptr<QLabel>(new QLabel(QString::number(i))));
      layout.addWidget(labels2.back().get());
   }
   // C++11 - QScopedPointer
   std::list<QScopedPointer<QLabel>> labels4;
   for (; i<15; ++i) {
      labels4.emplace_back(new QLabel(QString::number(i)));
      layout.addWidget(labels4.back().data());
   }
   // C++98 - boost
   boost::ptr_list<QLabel> labels3;
   for (; i<20; ++i) {
      labels3.push_back(new QLabel(QString::number(i)));
      layout.addWidget(&labels3.back());
   }
   // C++98 - QSharedPointer
   std::list<QSharedPointer<QLabel>> labels5;
   for (; i<25; ++i) {
      labels5.push_back(QSharedPointer<QLabel>(new QLabel(QString::number(i))));
      layout.addWidget(labels5.back().data());
   }

   w.show();
   return a.exec();
}
