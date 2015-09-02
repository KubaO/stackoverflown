#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>

#ifndef Q_OS_MAC
void disableMomentumScroll() {}
#else
extern "C" { void disableMomentumScroll(); }
#endif

float rnd(float range) { return (qrand() / static_cast<float>(RAND_MAX)) * range; }

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    disableMomentumScroll();
    QGraphicsScene s;
    for (int n = 0; n < 30; n++) {
        s.addRect(rnd(500), rnd(3000), rnd(200), rnd(1000), QPen(Qt::red), QBrush(Qt::gray));
    }
    QGraphicsView w(&s);
    w.show();
    return a.exec();
}
