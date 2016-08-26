// https://github.com/KubaO/stackoverflown/tree/master/questions/gscene-paint-20632209
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class EmptyItem : public QGraphicsItem
{
public:
    EmptyItem(QGraphicsItem * parent = nullptr) : QGraphicsItem{parent} {}
    QRectF boundingRect() const override { return {0, 0, 1, 1}; }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
};

class Scene : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(bool joinFigures READ joinFigures WRITE setJoinFigures)
    bool m_joinFigures = false;
    QGraphicsPathItem * m_item = nullptr;
    QPainterPath m_path;

    void newItem() {
        addItem(m_item = new QGraphicsPathItem);
        m_item->setPen(QPen{{qrand() % 256, qrand() % 256, qrand() % 256}});
        m_path = QPainterPath{}; // using std::swap; swap(m_path, QPainterPath());
    }
    void newPoint(const QPointF& pt) {
        if (! m_item) {
            newItem();
            m_path.moveTo(pt);
        } else {
            m_path.lineTo(pt);
            m_item->setPath(m_path);
        }
    }
    void mousePressEvent(QGraphicsSceneMouseEvent * ev) override {
        if (ev->buttons() != Qt::LeftButton) return;
        if (! m_joinFigures) m_item = nullptr;
        newPoint(ev->scenePos());
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override {
        if (ev->buttons() != Qt::LeftButton) return;
        newPoint(ev->scenePos());
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override {
        if (! m_path.isEmpty()) return;
        delete m_item; // Empty items are useless
        m_item = nullptr;
    }
public:
    Scene(QObject *parent = nullptr) : QGraphicsScene{parent}
    {
        addItem(new EmptyItem{});
    }
    Q_SLOT void setJoinFigures(bool j) { m_joinFigures = j; }
    bool joinFigures() const { return m_joinFigures; }
};

class Window : public QWidget
{
    Q_OBJECT
    QGridLayout m_layout{this};
    QGraphicsView m_view;
    QCheckBox m_join{"Join Figures (toggle with Spacebar)"};
    QAction m_toggleJoin{this};
public:
    Window(QWidget * parent = 0) : QWidget{parent}
    {
        m_layout.addWidget(&m_view);
        m_layout.addWidget(&m_join);
        m_view.setAlignment(Qt::AlignLeft | Qt::AlignTop);

        m_toggleJoin.setShortcut(QKeySequence(Qt::Key_Space));
        connect(&m_toggleJoin, SIGNAL(triggered()), &m_join, SLOT(toggle()));
        addAction(&m_toggleJoin);

        m_view.addAction(new QAction{"Clear", this});
        m_view.setContextMenuPolicy(Qt::ActionsContextMenu);
        connect(m_view.actions().at(0), SIGNAL(triggered()), SLOT(newScene()));

        // Create a new scene instead of clear()-ing it, since scenes can only grow their
        // sceneRect().
        newScene();
    }
    Q_SLOT void newScene() {
        if (m_view.scene()) m_view.scene()->deleteLater();
        m_view.setScene(new Scene);
        m_view.scene()->connect(&m_join, SIGNAL(toggled(bool)), SLOT(setJoinFigures(bool)));
    }
};

int main(int argc, char *argv[])
{
    QApplication a{argc, argv};
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
