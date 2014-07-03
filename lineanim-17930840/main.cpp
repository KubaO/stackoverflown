#include <QApplication>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPropertyAnimation>

class MyLine : public QGraphicsObject
{
public:
    MyLine(QGraphicsItem *parent = 0) : QGraphicsObject(parent) {}
    QRectF boundingRect() const { return QRectF(-60,-60,120,120); }
    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) {
        p->setPen(QPen(Qt::black, 8));
        p->drawEllipse(QPointF(0, 0), 50, 50);
        p->setPen(QPen(Qt::red, 8));
        p->drawLine(0, 0, 0, 50);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsObject * line = new MyLine;
    scene.addItem(line);
    QPropertyAnimation anim(line, "rotation");
    anim.setStartValue(0);
    anim.setEndValue(360);
    anim.setDuration(1000);
    anim.setLoopCount(-1); // forever
    anim.start();
    QGraphicsView view(&scene);
    view.setRenderHint(QPainter::Antialiasing);
    view.setSceneRect(-60, -60, 120, 120);
    view.show();
    return a.exec();
}

// old code

#if 0

#include <QApplication>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBasicTimer>

class MyLine : public QObject, public QGraphicsLineItem
{
    QBasicTimer m_timer;
    int m_i;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) {
            setLine(0, 0, 50+m_i, 50+m_i);
            m_i ++;
        }
    }
public:
    MyLine() : m_i(0) {
        setPen(QPen(Qt::red, 8));
        m_timer.start(100, this);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    scene.addItem(new MyLine);
    QGraphicsView view(&scene);
    view.setRenderHint(QPainter::Antialiasing);
    view.setSceneRect(0, 0, 300, 300);
    view.show();
    return a.exec();
}

#endif
