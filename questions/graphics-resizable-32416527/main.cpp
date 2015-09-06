// https://github.com/KubaO/stackoverflown/tree/master/questions/graphics-resizable-32416527
#include <QtWidgets>

class SimpleTraits {
public:
    /// Determines whether an item is manually resizeable.
    static bool isGraphicsItemResizeable(QGraphicsItem * item) {
        return dynamic_cast<QGraphicsEllipseItem*>(item);
    }
    /// Gives the rectangle one can base the resize operations on for an item
    static QRectF rectFor(QGraphicsItem * item) {
        auto ellipse = dynamic_cast<QGraphicsEllipseItem*>(item);
        if (ellipse) return ellipse->rect();
        return QRectF();
    }
    /// Sets a new rectangle on the item
    static void setRectOn(QGraphicsItem * item, const QRectF & rect) {
        auto ellipse = dynamic_cast<QGraphicsEllipseItem*>(item);
        if (ellipse) {
            ellipse->setRect(rect);
            return;
        }
    }
};

/// The set of edges intersecting a rectangle of given pen width
Qt::Edges edgesAt(const QPointF & p, const QRectF & r, qreal w) {
    Qt::Edges edges;
    auto hw = w / 2.0;
    if (QRectF(r.x()-hw, r.y()-hw, w, r.height()+w).contains(p)) edges |= Qt::LeftEdge;
    if (QRectF(r.x()+r.width()-hw, r.y()-hw, w, r.height()+w).contains(p)) edges |= Qt::RightEdge;
    if (QRectF(r.x()-hw, r.y()-hw, r.width()+w, w).contains(p)) edges |= Qt::TopEdge;
    if (QRectF(r.x()-hw, r.y()+r.height()-hw, r.width()+w, w).contains(p)) edges |= Qt::BottomEdge;
    return edges;
}

template <typename Tr>
class ResizeHelperItem : public QGraphicsObject {
    QRectF m_rect;
    QPen m_pen;
    Qt::Edges m_edges;
    void newGeometry() {
        prepareGeometryChange();
        auto parentRect = parentItem()->boundingRect();
        m_rect.setTopLeft(mapFromParent(parentRect.topLeft()));
        m_rect.setBottomRight(mapFromParent(parentRect.bottomRight()));
        m_pen.setWidthF(std::max(m_rect.width(), m_rect.height()) * 0.1);
        m_pen.setJoinStyle(Qt::MiterJoin);
    }
public:
    ResizeHelperItem() {
        setAcceptedMouseButtons(Qt::LeftButton);
        m_pen.setColor(QColor(255, 0, 0, 128));
        m_pen.setStyle(Qt::SolidLine);
    }
    QRectF boundingRect() const Q_DECL_OVERRIDE {
        auto hWidth = m_pen.widthF()/2.0;
        return m_rect.adjusted(-hWidth, -hWidth, hWidth, hWidth);
    }
    void selectionChanged() {
        if (!scene()) { setVisible(false); return; }
        auto sel = scene()->selectedItems();
        if (sel.isEmpty() || sel.size() > 1) { setVisible(false); return; }
        auto item = sel.at(0);
        if (! Tr::isGraphicsItemResizeable(item)) { setVisible(false); return; }
        setParentItem(item);
        newGeometry();
        setVisible(true);
    }
    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE {
        p->setPen(m_pen);
        p->drawRect(m_rect);
    }
    void mousePressEvent(QGraphicsSceneMouseEvent * ev) Q_DECL_OVERRIDE {
        m_edges = edgesAt(ev->pos(), m_rect, m_pen.widthF());
        if (!m_edges) return;
        ev->accept();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent * ev) Q_DECL_OVERRIDE {
        auto pos = mapToItem(parentItem()->parentItem(), ev->pos());
        auto newPos = parentItem()->pos();
        if (m_edges & Qt::LeftEdge)
            newPos.setX(pos.x());
        if (m_edges & Qt::TopEdge)
            newPos.setY(pos.y());
        if (m_edges & Qt::LeftEdge || m_edges & Qt::TopEdge)
            parentItem()->setPos(newPos);
        pos = mapToItem(parentItem(), ev->pos());
        auto rect = Tr::rectFor(parentItem());
        if (m_edges & Qt::RightEdge)
            rect.setRight(pos.x());
        if (m_edges & Qt::BottomEdge)
            rect.setBottom(pos.y());
        if (m_edges & Qt::RightEdge || m_edges & Qt::BottomEdge)
            Tr::setRectOn(parentItem(), rect);
        if (!!m_edges) newGeometry();
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QGraphicsScene scene;
    QGraphicsView view { &scene };
    typedef ResizeHelperItem<SimpleTraits> HelperItem;
    HelperItem helper;
    QObject::connect(&scene, &QGraphicsScene::selectionChanged, &helper, &HelperItem::selectionChanged);
    scene.addItem(&helper);
    auto item = scene.addEllipse(0, 0, 100, 100);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    view.setMinimumSize(400, 400);
    view.show();
    return app.exec();
}

