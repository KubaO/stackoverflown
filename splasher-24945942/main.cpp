#include <QApplication>
#include <QStateMachine>
#include <QTimer>
#include <QStackedWidget>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <random>

typedef std::default_random_engine random_engine;

QImage randomImage(int size, random_engine & rng) {
   QImage img(size, size, QImage::Format_ARGB32_Premultiplied);
   img.fill(Qt::white);
   QPainter p(&img);
   p.setRenderHint(QPainter::Antialiasing);
   int N = std::uniform_int_distribution<>(25, 200)(rng);
   std::uniform_real_distribution<> dP(0, size);
   std::uniform_int_distribution<> dC(0, 255);
   QPointF pt1(dP(rng), dP(rng));
   for (int i = 0; i < N; ++i) {
      QColor c(dC(rng), dC(rng), dC(rng));
      p.setPen(QPen(c, 3));
      QPointF pt2(dP(rng), dP(rng));
      p.drawLine(pt1, pt2);
      pt1 = pt2;
   }
   return img;
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   std::random_device rd;
   random_engine gen(rd());

   int imageSize = 300;
   QList<QImage> images;
   for (int n = 0; n < 28; ++n) images << randomImage(imageSize, gen);
   std::uniform_int_distribution<> dImage(0, images.size()-1);

   QStackedWidget display;
   QPushButton ready("I'm Ready!");
   QLabel label, labelHidden;
   display.addWidget(&ready);
   display.addWidget(&label);
   display.addWidget(&labelHidden);

   QTimer splashTimer;
   QStateMachine machine;
   QState s1(&machine), s2(&machine), s3(&machine), s4(&machine);
   splashTimer.setSingleShot(true);

   QObject::connect(&s1, &QState::entered, [&]{
      display.setCurrentWidget(&ready);
      ready.setDefault(true);
      ready.setFocus();
   });
   s1.addTransition(&ready, "clicked()", &s2);

   QObject::connect(&s2, &QState::entered, [&]{
      label.setPixmap(QPixmap::fromImage(images.at(dImage(gen))));
      display.setCurrentWidget(&label);
      splashTimer.start(250 + std::uniform_int_distribution<>(1500, 3000)(gen));
   });
   s2.addTransition(&splashTimer, "timeout()", &s3);

   QObject::connect(&s3, &QState::entered, [&]{
      display.setCurrentWidget(&labelHidden);
      splashTimer.start(2000);
   });
   s3.addTransition(&splashTimer, "timeout()", &s4);

   QObject::connect(&s4, &QState::entered, [&]{
      display.setCurrentWidget(&label);
      splashTimer.start(3000);
   });
   s4.addTransition(&splashTimer, "timeout()", &s1);

   machine.setInitialState(&s1);
   machine.start();
   display.show();

   return a.exec();
}
