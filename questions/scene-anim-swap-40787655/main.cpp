// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-anim-swap-40787655
#include <QtWidgets>
#include <cmath>

class QGraphicsRectWidget : public QGraphicsWidget
{
public:
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
   {
      painter->fillRect(rect(), Qt::blue);
      painter->setPen(Qt::yellow);
      painter->drawText(rect(), QString::number(zValue()), QTextOption(Qt::AlignCenter));
   }
};

class SwapAnimation : public QPropertyAnimation
{
   QPointer<QObject> other;
   qreal offset;
   QPoint propertyOf(QObject *obj) {
      return obj->property(propertyName().constData()).toPoint();
   }
   void updateState(State newState, State oldState) override {
      if (newState == Running && oldState == Stopped) {
         auto start = propertyOf(targetObject());
         auto end = propertyOf(other);
         auto step1 = fabs(offset);
         auto step2 = QLineF(start,end).length();
         auto steps = 2.0*step1 + step2;
         setStartValue(start);
         setKeyValueAt(step1/steps, QPoint(start.x(), start.y() + offset));
         setKeyValueAt((step1+step2)/steps, QPoint(end.x(), end.y() + offset));
         setEndValue(end);
         setDuration(10.0 * steps);
      }
      QPropertyAnimation::updateState(newState, oldState);
   }
public:
   SwapAnimation(QObject *first, QObject *second, qreal offset) : other(second), offset(offset) {
      setTargetObject(first);
      setPropertyName("pos");
   }
};

QParallelAnimationGroup* getSwapAnimation(QObject *obj1, QObject *obj2)
{
   auto const swapHeight = 75.0;
   auto par = new QParallelAnimationGroup;
   par->addAnimation(new SwapAnimation(obj2, obj1, -swapHeight));
   par->addAnimation(new SwapAnimation(obj1, obj2, swapHeight));
   return par;
}

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QGraphicsScene scene(0, 0, 300, 300);
   QGraphicsRectWidget buttons[4];
   int i = 0;
   QPointF start(20, 125);
   for (auto & button : buttons) {
      button.setZValue(i++);
      button.resize(50,50);
      button.setPos(start);
      start.setX(start.x() + 70);
      scene.addItem(&button);
   }

   QSequentialAnimationGroup gr;
   gr.addAnimation(getSwapAnimation(&buttons[0], &buttons[1]));
   gr.addAnimation(getSwapAnimation(&buttons[1], &buttons[2]));
   gr.addAnimation(getSwapAnimation(&buttons[2], &buttons[3]));
   gr.addAnimation(getSwapAnimation(&buttons[3], &buttons[1]));
   gr.start();

   QGraphicsView view(&scene);
   view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   view.resize(300, 300);
   view.show();
   return app.exec();
}

