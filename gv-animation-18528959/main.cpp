#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QGraphicsRectItem>

class EmptyGraphicsObject : public QGraphicsObject
{
public:
    EmptyGraphicsObject() {}
    QRectF boundingRect() const { return QRectF(0, 0, 0, 0); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}
};

class View : public QGraphicsView
{
public:
    View(QGraphicsScene *scene, QWidget *parent = 0) : QGraphicsView(scene, parent) {
        setRenderHint(QPainter::Antialiasing);
    }
    void resizeEvent(QResizeEvent *) {
        fitInView(-2, -2, 4, 4, Qt::KeepAspectRatio);
    }
};

void setupScene(QGraphicsScene &s)
{
    QGraphicsObject * obj = new EmptyGraphicsObject;
    QGraphicsRectItem * rect = new QGraphicsRectItem(-1, 0.3, 2, 0.3, obj);
    QPropertyAnimation * anim = new QPropertyAnimation(obj, "rotation", &s);
    s.addItem(obj);
    rect->setPen(QPen(Qt::darkBlue, 0.1));
    anim->setDuration(2000);
    anim->setStartValue(0);
    anim->setEndValue(360);
    anim->setEasingCurve(QEasingCurve::InBounce);
    anim->setLoopCount(-1);
    anim->start();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene s;
    setupScene(s);
    View v(&s);
    v.show();
    return a.exec();
}

