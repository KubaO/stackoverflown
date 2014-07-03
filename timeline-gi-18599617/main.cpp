#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsObject>
#include <QPainter>
#include <QApplication>

class Item : public QGraphicsObject {
    Q_OBJECT
    qreal m_pen;
    QSizeF m_size;
    Q_PROPERTY(QSizeF size READ size WRITE setSize NOTIFY newSize)
    Q_SIGNAL void newSize();
    qreal width() const { return m_size.width() + m_pen; }
    qreal height() const { return m_size.height() + m_pen; }
public:
    explicit Item(QGraphicsItem *parent = 0) :
        QGraphicsObject(parent), m_pen(5.0), m_size(50.0, 50.0) {}
    QSizeF size() const { return m_size; }
    void setSize(const QSizeF & size) {
        if (m_size != size) {
            m_size = size;
            update();
            emit newSize();
        }
    }
    QRectF boundingRect() const {
        return QRectF(-width()/2, -height()/2, width(), height());
    }
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
        p->setPen(QPen(Qt::black, m_pen));
        p->drawEllipse(QPointF(0,0), width()/2, height()/2);
        p->setPen(QPen(Qt::red, m_pen));
        p->drawLine(0, 0, 0, height()/2);
    }
};

void animate(QObject * obj)
{
    QParallelAnimationGroup * group = new QParallelAnimationGroup(obj);
    QPropertyAnimation * pos = new QPropertyAnimation(obj, "pos", obj);
    QPropertyAnimation * size = new QPropertyAnimation(obj, "size", obj);
    QPropertyAnimation * rot= new QPropertyAnimation(obj, "rotation", obj);
    pos->setDuration(3000);
    pos->setLoopCount(-1);
    pos->setEasingCurve(QEasingCurve::InOutCubic);
    pos->setStartValue(QPointF(-50, -50));
    pos->setEndValue(QPointF(50, 50));
    size->setDuration(1500);
    size->setLoopCount(-1);
    size->setEasingCurve(QEasingCurve::InOutElastic);
    size->setStartValue(QSizeF(100, 100));
    size->setEndValue(QSizeF(100, 30));
    rot->setDuration(1000);
    rot->setLoopCount(-1);
    rot->setStartValue(0.0);
    rot->setEndValue(360.0);
    group->addAnimation(pos);
    group->addAnimation(size);
    group->addAnimation(rot);
    group->start();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene s;
    QGraphicsView v(&s);
    Item * item = new Item;
    s.addItem(item);
    v.setRenderHint(QPainter::Antialiasing);
    v.setSceneRect(-125, -125, 300, 300);
    v.show();
    animate(item);
    return a.exec();
}

#include "main.moc"
