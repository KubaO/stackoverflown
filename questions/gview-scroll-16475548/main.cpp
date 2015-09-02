//main.cpp
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtCore/qmath.h>
#include <QScrollBar>
#include <QWheelEvent>
#include <QDebug>

class View : public QGraphicsView {
public:
    void zoom(int delta) {
        double factor = qPow(1.2, delta/qAbs(delta));
        scale(factor, factor);
    }
    void wheelEvent(QWheelEvent *event) {
        if (event->modifiers() & Qt::ControlModifier) {
            zoom(event->delta());
        }
        else if (event->modifiers() & Qt::ShiftModifier) {
            horizontalScrollBar()->event(event);
        }
        else {
            QGraphicsView::wheelEvent(event);
        }
    }
public:
    explicit View(QWidget *parent=0) : QGraphicsView(parent) {}
    explicit View(QGraphicsScene *scene, QWidget *parent=0) : QGraphicsView(scene, parent) {}
};

#ifndef Q_OS_MAC
void disableMomentumScroll() {}
#else
extern "C" { void disableMomentumScroll(); }
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    disableMomentumScroll();
    QGraphicsScene s;
    s.addEllipse(-50, -50, 100, 100, QPen(Qt::red), QBrush(Qt::gray));
    View w(&s);
    w.show();
    return a.exec();
}
