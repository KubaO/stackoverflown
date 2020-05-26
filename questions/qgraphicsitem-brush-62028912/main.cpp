// https://github.com/KubaO/stackoverflown/tree/master/questions/qgraphicsitem-brush-62028912
#include <QtWidgets>
#include <cstdlib>

QColor randColor() { return QRgb((rand() << 16) ^ rand()); }

int main(int argc, char *argv[])
{
   srand(QDateTime::currentDateTime().toMSecsSinceEpoch());
   QApplication a(argc, argv);
   QGraphicsScene scene;
   QGraphicsView view(&scene);

   auto *rect = scene.addRect(0, 0, 100, 100);
   view.fitInView(rect);

   QTimer timer;
   QObject::connect(&timer, &QTimer::timeout, &view, [rect]{
      rect->setBrush(randColor());
   });
   timer.start(500);

   view.show();
   return a.exec();
}
