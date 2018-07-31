// https://github.com/KubaO/stackoverflown/tree/master/questions/graphicsscene-children-51596611
#include <QtWidgets>
#include <array>
#define SOLUTION(s) ((!!(s)) << (s))
#define HAS_SOLUTION(s) (!!(SOLUTIONS & SOLUTION(s)))
#define SOLUTIONS (SOLUTION(1) | SOLUTION(2) | SOLUTION(3))

class SizeGripItem : public QGraphicsObject {
   Q_OBJECT
   enum { kMoveInHandle, kInitialPos, kPressPos };
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
#if HAS_SOLUTION(2)
      bool sceneEvent(QEvent *event) override {
         return (data(kMoveInHandle).toBool() && hasSelectedMovableAncestor(this) &&
                 processMove(this, event)) ||
                QGraphicsRectItem::sceneEvent(event);
      }
#endif
   };
#if HAS_SOLUTION(2) || HAS_SOLUTION(3)
   static bool processMove(QGraphicsItem *item, QEvent *ev) {
      auto mev = static_cast<QGraphicsSceneMouseEvent *>(ev);
      if (ev->type() == QEvent::GraphicsSceneMousePress &&
          mev->button() == Qt::LeftButton) {
         item->setData(kInitialPos, item->pos());
         item->setData(kPressPos, item->mapToParent(mev->pos()));
         return true;
      } else if (ev->type() == QEvent::GraphicsSceneMouseMove &&
                 mev->buttons() == Qt::LeftButton) {
         auto delta = item->mapToParent(mev->pos()) - item->data(kPressPos).toPointF();
         item->setPos(item->data(kInitialPos).toPointF() + delta);
         return true;
      }
      return false;
   }
   static bool hasSelectedMovableAncestor(const QGraphicsItem *item) {
      auto *p = item->parentItem();
      return p && ((p->isSelected() && (p->flags() & QGraphicsItem::ItemIsMovable)) ||
                   hasSelectedMovableAncestor(p));
   }
#endif
   std::array<HandleItem, 4> handles_;
   QRectF rect_;
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
   SizeGripItem(QGraphicsItem *parent = {}) : QGraphicsObject(parent) {
      for (auto &h : handles_) h.setParentItem(this);
      setFlags(ItemHasNoContents);
   }
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
      if (change == QGraphicsItem::ItemPositionHasChanged) resize();
      return value;
   }
   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
   QRectF boundingRect() const override { return rect_; }
   void setRect(const QRectF &rect) {
      rect_ = mapRectFromParent(rect);
      resize();
      updateHandleItemPositions();
   }
   void resize() { emit rectChanged(mapRectToParent(rect_), parentItem()); }
   Q_SIGNAL void rectChanged(const QRectF &, QGraphicsItem *);
#if SOLUTIONS
   void selectSolution(int i) {
#if HAS_SOLUTION(1)
      setFlag(ItemIsMovable, i == 1);
      setFlag(ItemSendsGeometryChanges, i == 1);
      if (i != 1) {
         auto rect = mapRectToParent(rect_);
         setPos({});  // reset position if we're leaving the movable mode
         setRect(rect);
      }
      i--;
#endif
      for (auto &h : handles_) {
         int ii = i;
#if HAS_SOLUTION(2)
         h.setData(kMoveInHandle, ii-- == 1);
#endif
#if HAS_SOLUTION(3)
         if (ii == 1)
            h.installSceneEventFilter(this);
         else
            h.removeSceneEventFilter(this);
#endif
      }
   }
#endif
#if HAS_SOLUTION(3)
   bool sceneEventFilter(QGraphicsItem *item, QEvent *ev) override {
      if (hasSelectedMovableAncestor(item)) return processMove(item, ev);
      return false;
   }
#endif
};

class SignalingBoxItem : public QObject, public QGraphicsRectItem {
   Q_OBJECT
   SizeGripItem m_sizeGrip{this};
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
      if (change == QGraphicsItem::ItemSelectedHasChanged)
         m_sizeGrip.setVisible(value.toBool());
      else if (change == QGraphicsItem::ItemScenePositionHasChanged)
         emitRectChanged();
      return value;
   }
   void emitRectChanged() { emit rectChanged(mapRectToScene(rect())); }
   void setRectImpl(const QRectF &rect) {
      QGraphicsRectItem::setRect(rect);
      emitRectChanged();
   }

  public:
   SignalingBoxItem(const QRectF &rect = {}, QGraphicsItem *parent = {})
       : QGraphicsRectItem(rect, parent) {
      setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
      m_sizeGrip.hide();
      connect(&m_sizeGrip, &SizeGripItem::rectChanged, this,
              &SignalingBoxItem::setRectImpl);
   }
   void setRect(const QRectF &rect) {
      setSelected(false);
      m_sizeGrip.setRect(rect);
      setRectImpl(rect);
   }
   Q_SIGNAL void rectChanged(const QRectF &);  // Rectangle in scene coordinates
#if SOLUTIONS
   void selectSolution(int index) {
      setFlag(ItemIsMovable, !HAS_SOLUTION(1) || index != 1);
      m_sizeGrip.selectSolution(index);
   }
#endif
};

class SampleEditor : public QGraphicsView {
   Q_OBJECT
   bool m_activeDrag = false;
   SignalingBoxItem m_box;
   QPointF m_dragStart;

  public:
   SampleEditor(QGraphicsScene *scene) : QGraphicsView(scene) {
      scene->addItem(&m_box);
      connect(&m_box, &SignalingBoxItem::rectChanged, this, &SampleEditor::rectChanged);
   }
   Q_SIGNAL void rectChanged(const QRectF &);
   void mousePressEvent(QMouseEvent *event) override {
      QGraphicsView::mousePressEvent(event);
      if (event->button() == Qt::RightButton) {
         m_dragStart = m_box.mapFromScene(mapToScene(event->pos()));
         m_activeDrag = true;
         m_box.show();
         m_box.setRect({m_dragStart, m_dragStart});
         event->accept();
      }
   }
   void mouseMoveEvent(QMouseEvent *event) override {
      QGraphicsView::mouseMoveEvent(event);
      if (m_activeDrag) {
         m_box.setRect({m_dragStart, m_box.mapFromScene(mapToScene(event->pos()))});
         event->accept();
      }
   }
   void mouseReleaseEvent(QMouseEvent *event) override {
      QGraphicsView::mouseReleaseEvent(event);
      if (m_activeDrag && event->button() == Qt::RightButton) {
         event->accept();
         m_activeDrag = false;
      }
   }
   void resizeEvent(QResizeEvent *event) override {
      QGraphicsView::resizeEvent(event);
      scene()->setSceneRect(contentsRect());
   }
#if SOLUTIONS
   void selectSolution(int index) { m_box.selectSolution(index); }
#endif
};

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QWidget ui;
   QGridLayout layout{&ui};
   QGraphicsScene scene;
   SampleEditor editor(&scene);
   QComboBox sel;
   QLabel status;
   layout.addWidget(&editor, 0, 0, 1, 2);
   layout.addWidget(&sel, 1, 0);
   layout.addWidget(&status, 1, 1);
   sel.addItems({
      "Original (Movable SignalingBoxItem)",
#if HAS_SOLUTION(1)
          "Movable SizeGripItem",
#endif
#if HAS_SOLUTION(2)
          "Reimplemented HandleItem",
#endif
#if HAS_SOLUTION(3)
          "Filtering SizeGripItem",
#endif
   });
   sel.setCurrentIndex(-1);
#if SOLUTIONS
   QObject::connect(&sel, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    [&](int index) { editor.selectSolution(index); });
#endif
   QObject::connect(&editor, &SampleEditor::rectChanged, &status,
                    [&](const QRectF &rect) {
                       QString s;
                       QDebug(&s) << rect;
                       status.setText(s);
                    });
   sel.setCurrentIndex((sel.count() > 1) ? 1 : 0);
   ui.setMinimumSize(640, 480);
   ui.show();
   return a.exec();
}
#include "main.moc"
