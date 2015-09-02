#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>

class XRectItem : public QGraphicsRectItem {
   void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *wdg = 0)
   Q_DECL_OVERRIDE
   {
      QGraphicsRectItem::paint(p, opt, wdg);
      p->drawEllipse(rect());
   }
public:
   XRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem * parent = 0) :
      QGraphicsRectItem(x, y, w, h, parent) {}
};

class View : public QGraphicsView
{
   void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE {
      fitInView(sceneRect(), Qt::KeepAspectRatio);
   }
public:
   View(QGraphicsScene *scene, QWidget *parent = 0) :
      QGraphicsView(scene, parent) {}
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QGraphicsScene s;
   XRectItem rect(-1.5, 1.5, 3, 2);
   s.addItem(&rect);
   QTimer timer;
   timer.start(100);
   QObject::connect(&timer, &QTimer::timeout, [&rect]{
      rect.setPen(QPen(QColor(rand() % 256, rand() % 256, rand() % 256), 0.1));
   });
   View v(&s);
   v.setRenderHint(QPainter::Antialiasing);
   v.show();
   return a.exec();
}
