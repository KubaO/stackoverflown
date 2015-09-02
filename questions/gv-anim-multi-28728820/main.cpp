#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGridLayout>
#include <QTime>
#include <QTimer>
#include <array>

class View : public QGraphicsView
{
public:
   View(QWidget *parent = 0) : QGraphicsView(parent) {
      setRenderHint(QPainter::Antialiasing);
   }
   void resizeEvent(QResizeEvent *) {
      fitInView(-1, -1, 2, 2, Qt::KeepAspectRatio);
   }
};

template <typename Container>
void updateScenes(Container & views)
{
   auto angle = 360.0/1000.0 * (QTime::currentTime().msecsSinceStartOfDay() % 1000);
   for (auto & view : views) {
      auto scene = view.scene();
      scene->clear();
      auto * line = scene->addLine(-1, 0, 1, 0, QPen(Qt::darkBlue, 0.1));
      line->setRotation(angle);
   }
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QGraphicsScene s;
   QTimer timer;
   QWidget window;
   QGridLayout layout(&window);
   std::array<View, 64> views;

   int i = 0;
   for (auto & view : views) {
      view.setScene(new QGraphicsScene(&view));
      layout.addWidget(&view, i/8, i%8);
      ++ i;
   }

   QObject::connect(&timer, &QTimer::timeout, [&views]{ updateScenes(views); });
   timer.start(50);
   window.show();
   return a.exec();
   a.
}
