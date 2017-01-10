// https://github.com/KubaO/stackoverflown/tree/master/questions/graphics-transform-32186798
#include <QtWidgets>

struct Controller {
public:
   QSlider angle, xScale, yScale;
   Controller(QGridLayout & grid, int col) {
      angle.setRange(-180, 180);
      xScale.setRange(1, 10);
      yScale.setRange(1, 10);
      grid.addWidget(&angle, 0, col + 0);
      grid.addWidget(&xScale, 0, col + 1);
      grid.addWidget(&yScale, 0, col + 2);
   }
   template <typename F> void connect(F && f) { connect(f, f, std::forward<F>(f)); }
   template <typename Fa, typename Fx, typename Fy> void connect(Fa && a, Fx && x, Fy && y) {
      QObject::connect(&angle, &QSlider::valueChanged, std::forward<Fa>(a));
      QObject::connect(&xScale, &QSlider::valueChanged, std::forward<Fx>(x));
      QObject::connect(&yScale, &QSlider::valueChanged, std::forward<Fy>(y));
   }
   QTransform xform(QPointF xlate) {
      QTransform t;
      t.translate(xlate.x(), xlate.y());
      t.rotate(angle.value());
      t.scale(xScale.value(), yScale.value());
      t.translate(-xlate.x(), -xlate.y());
      return t;
   }
};

int main(int argc, char **argv)
{
   auto text = QStringLiteral("Hello, World!");
   QApplication app(argc, argv);
   QGraphicsScene scene;
   QWidget w;
   QGridLayout layout(&w);
   QGraphicsView view(&scene);
   Controller left(layout, 0), right(layout, 4);
   layout.addWidget(&view, 0, 3);

   auto ref = new QGraphicsTextItem(text);         // a reference, not resized
   ref->setDefaultTextColor(Qt::red);
   ref->setTransformOriginPoint(ref->boundingRect().center());
   ref->setRotation(45);
   scene.addItem(ref);

   auto leftItem = new QGraphicsTextItem(text);    // controlled from the left
   leftItem->setDefaultTextColor(Qt::green);
   scene.addItem(leftItem);

   auto rightItem = new QGraphicsTextItem(text);   // controlled from the right
   rightItem->setDefaultTextColor(Qt::blue);
   scene.addItem(rightItem);

   QGraphicsRotation rot;
   QGraphicsScale scale;
   rightItem->setTransformations(QList<QGraphicsTransform*>() << &rot << &scale);
   rot.setOrigin(QVector3D(rightItem->boundingRect().center()));
   scale.setOrigin(QVector3D(rightItem->boundingRect().center()));

   left.connect([leftItem, &left]{ leftItem->setTransform(left.xform(leftItem->boundingRect().center()));});
   right.connect([&rot](int a){ rot.setAngle(a); },
                 [&scale](int s){ scale.setXScale(s); }, [&scale](int s){ scale.setYScale(s); });
   right.angle.setValue(45);
   right.xScale.setValue(3);
   right.yScale.setValue(1);

   view.ensureVisible(scene.sceneRect());
   w.show();
   return app.exec();
}
