// https://github.com/KubaO/stackoverflown/tree/master/questions/scenemod-11232425
#include <QtWidgets>

const char kNotifier[] = "notifier";

class Notifier : public QObject
{
   Q_OBJECT
   int m_count = {};
public:
   int count() const { return m_count; }
   void inc() { m_count ++; }
   void notify() { m_count = {}; emit notification(); }
   Q_SIGNAL void notification();
};

typedef QPointer<Notifier> NotifierPointer;
Q_DECLARE_METATYPE(NotifierPointer)

template <typename T> class NotifyingItem : public T
{
protected:
   QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override {
      QVariant v;
      if (change == T::ItemPositionHasChanged &&
          this->scene() &&
          (v=this->scene()->property(kNotifier)).isValid())
      {
         auto notifier = v.value<NotifierPointer>();
         notifier->inc();
         if (notifier->count() >= this->scene()->selectedItems().count()) {
            notifier->notify();
         }
      }
      return T::itemChange(change, value);
   }
};

// Note that all you need to make Circle a notifying item is to derive from
// NotifyingItem<basetype>.

class Circle : public NotifyingItem<QGraphicsEllipseItem>
{
   QBrush m_brush;
public:
   Circle(const QPointF & c) : m_brush(Qt::lightGray) {
      const qreal r = 10.0 + (50.0*qrand())/RAND_MAX;
      setRect({-r, -r, 2.0*r, 2.0*r});
      setPos(c);
      setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
               QGraphicsItem::ItemSendsGeometryChanges);
      setPen({Qt::red});
      setBrush(m_brush);
   }
};

class View : public QGraphicsView
{
   Q_OBJECT
   QGraphicsScene scene;
   QGraphicsSimpleTextItem text;
   QGraphicsRectItem centroid{-5, -5, 10, 10};
   Notifier notifier;
   int deltaCounter = {};
public:
   explicit View(QWidget *parent = {});
protected:
   Q_SLOT void gotUpdates();
   void mousePressEvent(QMouseEvent *event) override;
};

View::View(QWidget *parent) : QGraphicsView(parent)
{
   centroid.hide();
   centroid.setRotation(45.0);
   centroid.setPen({Qt::blue});
   centroid.setZValue(2);
   scene.addItem(&centroid);
   text.setPos(5, 470);
   text.setZValue(1);
   scene.addItem(&text);
   setRenderHint(QPainter::Antialiasing);
   setScene(&scene);
   setSceneRect(0,0,500,500);
   scene.setProperty(kNotifier, QVariant::fromValue(NotifierPointer(&notifier)));
   connect(&notifier, &Notifier::notification, this, &View::gotUpdates);
   connect(&scene, &QGraphicsScene::selectionChanged, &notifier, &Notifier::notification);
}

void View::gotUpdates()
{
   if (scene.selectedItems().isEmpty()) {
      centroid.hide();
      return;
   }
   centroid.show();
   QPointF centroid;
   qreal area = {};
   for (auto item : scene.selectedItems()) {
      const QRectF r = item->boundingRect();
      const qreal a = r.width() * r.height();
      centroid += item->pos() * a;
      area += a;
   }
   if (area > 0) centroid /= area;
   auto st = QStringLiteral("delta #%1 with %2 items, centroid at %3, %4")
         .arg(deltaCounter++).arg(scene.selectedItems().count())
         .arg(centroid.x(), 0, 'f', 1).arg(centroid.y(), 0, 'f', 1);
   this->centroid.setPos(centroid);
   text.setText(st);
}

void View::mousePressEvent(QMouseEvent *event)
{
   const auto center = mapToScene(event->pos());
   if (! scene.itemAt(center, {})) scene.addItem(new Circle{center});
   QGraphicsView::mousePressEvent(event);
}

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   View v;
   v.show();
   return app.exec();
}
#include "main.moc"
