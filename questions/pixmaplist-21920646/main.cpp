#include <QGuiApplication>
#include <QPixmap>
#include <QPainter>

int main(int argc, char *argv[])
{
   QGuiApplication a(argc, argv);
   QList<QPixmap> pixmapList;
   for (int i=0;i<50;++i){
      QPixmap pixmap = QPixmap(1000,1000);
      pixmap.fill(Qt::transparent);
      pixmapList<<pixmap;
   }
   QPainter painter(&pixmapList[10]);
   painter.drawLine(0, 0, 100, 100);
   return 0;
}
