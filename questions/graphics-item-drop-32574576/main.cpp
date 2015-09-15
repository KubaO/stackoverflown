// VS2010, Qt 4.7
// https://github.com/KubaO/stackoverflown/tree/master/questions/graphics-item-drop-32574576
#include <QtWidgets>

class ucFilter : public QGraphicsRectItem {
   QGraphicsTextItem m_text;
public:
   ucFilter(const QString &name, const QPointF &pos, QGraphicsItem * parent = 0) :
      QGraphicsRectItem(parent),
      m_text(this)
   {
      static const QRect defaultRect(0, 0, 160, 80);
      setPos(pos);
      setRect(QRect(-defaultRect.topLeft()/2, defaultRect.size()));
      setFlags(QGraphicsItem::ItemIsMovable);
      setName(name);
   }
   void setName(const QString & text) {
      m_text.setPlainText(text);
      m_text.setPos(-m_text.boundingRect().width()/2, -30);
   }
   QString name() const {
      return m_text.toPlainText();
   }
};

const char * kMimeType = "application/x-qabstractitemmodeldatalist";

QVariant decode(const QMimeData* data, Qt::ItemDataRole role = Qt::DisplayRole) {
   auto buf = data->data(kMimeType);
   QDataStream stream(&buf, QIODevice::ReadOnly);
   while (!stream.atEnd()) {
      int row, col;
      QMap<int, QVariant> map;
      stream >> row >> col >> map;
      if (map.contains(role)) return map[role];
   }
   return QVariant();
}

class cGraphicsScene : public QGraphicsScene {
   QList<ucFilter*> m_filters;
   ucFilter* m_dragItem;
public:
   cGraphicsScene(QObject * parent = 0) : QGraphicsScene(parent), m_dragItem(nullptr) {}
   void dragEnterEvent(QGraphicsSceneDragDropEvent * event) Q_DECL_OVERRIDE {
      if (!event->mimeData()->hasFormat(kMimeType)) return;
      auto name = decode(event->mimeData()).toString();
      if (name.isEmpty()) return;
      QScopedPointer<ucFilter> filter(new ucFilter(name, event->scenePos()));
      addItem(m_dragItem = filter.take());
      event->acceptProposedAction();
   }
   void dragMoveEvent(QGraphicsSceneDragDropEvent * event) Q_DECL_OVERRIDE {
      if (!m_dragItem) return;
      m_dragItem->setPos(event->scenePos());
      event->acceptProposedAction();
   }
   void dropEvent(QGraphicsSceneDragDropEvent * event) Q_DECL_OVERRIDE {
      if (!m_dragItem) return;
      m_dragItem->setPos(event->scenePos());
      m_filters << m_dragItem;
      event->acceptProposedAction();
   }
   void dragLeaveEvent(QGraphicsSceneDragDropEvent * event) Q_DECL_OVERRIDE {
      delete m_dragItem;
      m_dragItem = nullptr;
      event->acceptProposedAction();
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   cGraphicsScene scene;
   QGraphicsView view(&scene);
   QListWidget list;
   QHBoxLayout l(&w);
   l.addWidget(&view);
   l.addWidget(&list);

   list.setFixedWidth(120);
   list.addItem("Item1");
   list.addItem("Item2");
   list.setDragDropMode(QAbstractItemView::DragOnly);
   view.setAcceptDrops(true);

   w.resize(500, 300);
   w.show();
   return app.exec();
}

