// https://github.com/KubaO/stackoverflown/tree/master/questions/graphicsscene-children-51596611
#include <QtWidgets>
#include <array>
#include <memory>

class SizeGripItem : public QGraphicsItem {
   struct HandleItem : QGraphicsRectItem {
      HandleItem() : QGraphicsRectItem(-4, -4, 8, 8) {
         setBrush(Qt::lightGray);
         setFlags(ItemIsMovable | ItemSendsGeometryChanges);
      }
      SizeGripItem *parent() const { return static_cast<SizeGripItem *>(parentItem()); }
      QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
         if (change == ItemPositionHasChanged) parent()->handleMoved(this);
         return value;
      }
   };
   using Resizer = std::function<void(QGraphicsItem *, const QRectF &)>;
   std::array<HandleItem, 4> handles_;
   QRectF rect_;
   Resizer resizer_;
   void updateHandleItemPositions() {
      static auto get = {&QRectF::topLeft, &QRectF::topRight, &QRectF::bottomLeft,
                         &QRectF::bottomRight};
      for (auto &h : handles_) h.setPos((rect_.*get.begin()[index(&h)])());
   }
   int index(HandleItem *handle) const { return handle - &handles_[0]; }
   void handleMoved(HandleItem *handle) {
      static auto set = {&QRectF::setTopLeft, &QRectF::setTopRight,
                         &QRectF::setBottomLeft, &QRectF::setBottomRight};
      auto rect = rect_;
      (rect.*set.begin()[index(handle)])(handle->pos());
      setRect(mapRectToParent(rect.normalized()));
   }

  public:
   SizeGripItem(Resizer r, QGraphicsItem *parent) : QGraphicsItem(parent), resizer_(r) {
      setFlags(/*ItemIsMovable | ItemSendsGeometryChanges |*/ ItemHasNoContents);
      for (auto &h : handles_) h.setParentItem(this);
   }
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
      if (change == QGraphicsItem::ItemPositionHasChanged) resize();
      return value;
   }
   QRectF boundingRect() const override { return rect_; }
   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
   void setRect(const QRectF &rect) {
      rect_ = mapRectFromParent(rect);
      resize();
      updateHandleItemPositions();
   }
   void resize() {
      if (resizer_) resizer_(parentItem(), mapRectToParent(rect_));
   }
};

class SignalingBoxItem : public QObject, public QGraphicsRectItem {
   Q_OBJECT
   SizeGripItem m_sizeGrip{resizer, this};
   static void resizer(QGraphicsItem *item, const QRectF &rect) {
      static_cast<SignalingBoxItem *>(item)->setRect(rect);
   }
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
      if (change == QGraphicsItem::ItemSelectedHasChanged) {
         m_sizeGrip.setRect(rect());
         m_sizeGrip.setVisible(value.toBool());
      } else if (change == QGraphicsItem::ItemScenePositionHasChanged)
         emitRectChanged();
      return value;
   }
   void emitRectChanged() { emit rectChanged(mapRectToScene(rect())); }

  public:
   SignalingBoxItem(const QRectF &rect = {}, QGraphicsItem *parent = {})
       : QGraphicsRectItem(rect, parent) {
      setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
               QGraphicsItem::ItemSendsScenePositionChanges);
      m_sizeGrip.hide();
   }
   void setRect(const QRectF &rect) {
      QGraphicsRectItem::setRect(rect);
      qDebug() << boundingRect();
      emitRectChanged();
   }
   /// Rectangle in scene coordinates
   Q_SIGNAL void rectChanged(const QRectF &);
};

class SampleEditor : public QGraphicsView {
   Q_OBJECT
   bool m_activeDrag = false;
   QPointer<SignalingBoxItem> m_box;
   QPointF m_dragStart;

  public:
   using base = QGraphicsView;
   using QGraphicsView::QGraphicsView;

   void mousePressEvent(QMouseEvent *event) override {
      qDebug() << event;
      QGraphicsView::mousePressEvent(event);
      qDebug() << event;
      if (event->buttons() == Qt::RightButton) {
         m_dragStart = mapToScene(event->pos());
         if (!m_box) {
            m_box = new SignalingBoxItem;
            scene()->addItem(m_box);
            connect(m_box, &SignalingBoxItem::rectChanged, this,
                    &SampleEditor::rectChanged);
         }
         m_activeDrag = true;
         m_box->show();
         m_box->setRect({m_dragStart, m_dragStart});
         event->accept();
      }
   }
   void mouseMoveEvent(QMouseEvent *event) override {
      qDebug() << event;
      QGraphicsView::mouseMoveEvent(event);
      if (m_activeDrag && m_box) {
         m_box->setRect({m_dragStart, mapToScene(event->pos())});
         event->accept();
      }
   }
   void mouseReleaseEvent(QMouseEvent *event) override {
      QGraphicsView::mouseReleaseEvent(event);
      if (m_activeDrag && m_box) {
         emit rectChanged(m_box->rect());
         event->accept();
      }
      m_activeDrag = false;
   }
   void resizeEvent(QResizeEvent *event) override {
      QGraphicsView::resizeEvent(event);
      scene()->setSceneRect(contentsRect());
   }
   Q_SIGNAL void rectChanged(const QRectF &);
};

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QWidget ui;
   QVBoxLayout layout{&ui};
   QGraphicsScene scene;
   SampleEditor editor(&scene);
   QLabel status;
   layout.addWidget(&editor);
   layout.addWidget(&status);
   QObject::connect(&editor, &SampleEditor::rectChanged, &status,
                    [&](const QRectF &rect) {
                       QString s;
                       QDebug(&s) << rect;
                       status.setText(s);
                    });
   ui.setMinimumSize(640, 480);
   ui.show();
   return a.exec();
}
#include "main.moc"
