#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCheckBox>
#include <QAction>
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

class EmptyItem : public QGraphicsItem
{
public:
    EmptyItem(QGraphicsItem * parent = 0) : QGraphicsItem(parent) {}
    QRectF boundingRect() const { return QRectF(0, 0, 1, 1); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}
};

class Scene : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(bool joinFigures READ joinFigures WRITE setJoinFigures)
    bool m_joinFigures;
    QGraphicsPathItem * m_item;
    QPainterPath m_path;

    void newItem() {
        addItem(m_item = new QGraphicsPathItem);
        m_item->setPen(QPen(QColor(qrand() % 256, qrand() % 256, qrand() % 256)));
        m_path = QPainterPath(); // std::swap(m_path, QPainterPath());
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
    void mousePressEvent(QGraphicsSceneMouseEvent * ev) {
        if (ev->buttons() != Qt::LeftButton) return;
        if (! m_joinFigures) m_item = 0;
        newPoint(ev->scenePos());
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) {
        if (ev->buttons() != Qt::LeftButton) return;
        newPoint(ev->scenePos());
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
        if (! m_path.isEmpty()) return;
        delete m_item; // Empty items are useless
        m_item = 0;
    }
public:
    Scene(QObject *parent = 0) : QGraphicsScene(parent),
        m_joinFigures(false), m_item(0)
    {
        addItem(new EmptyItem());
    }
    Q_SLOT void setJoinFigures(bool j) { m_joinFigures = j; }
    bool joinFigures() const { return m_joinFigures; }
};

class Window : public QWidget
{
    Q_OBJECT
    QGraphicsView * m_view;
    QCheckBox * m_join;
public:
    Window(QWidget * parent = 0) :
        QWidget(parent),
        m_view(new QGraphicsView),
        m_join(new QCheckBox("Join Figures (toggle with Spacebar)"))
    {
        QGridLayout * layout = new QGridLayout(this);
        layout->addWidget(m_view);
        layout->addWidget(m_join);
        m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        QAction * toggleJoin = new QAction(this);
        toggleJoin->setShortcut(QKeySequence(Qt::Key_Space));
        connect(toggleJoin, SIGNAL(triggered()), m_join, SLOT(toggle()));
        addAction(toggleJoin);

        m_view->addAction(new QAction("Clear", this));
        m_view->setContextMenuPolicy(Qt::ActionsContextMenu);
        QObject::connect(m_view->actions().at(0), SIGNAL(triggered()), SLOT(newScene()));

        // Create a new scene instead of clear()-ing it, since scenes can only grow their
        // sceneRect().
        newScene();
    }
    Q_SLOT void newScene() {
        QGraphicsScene * oldScene = m_view->scene();
        m_view->setScene(new Scene);
        m_view->scene()->connect(m_join, SIGNAL(toggled(bool)), SLOT(setJoinFigures(bool)));
        delete oldScene;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
