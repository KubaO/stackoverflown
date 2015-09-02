#include <QApplication>
#include <QMainWindow>
#include <QPainter>

class Bottom {
public:
    void paint(QPainter * p) {
        p->setBrush(Qt::blue);
        p->drawRect(0, 0, 1000, 1000);
    }
};

class Top : public QWidget {
    Bottom * m_bottom;
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::red);
        p.drawRect(0, 0, 100, 100);
        m_bottom->paint(&p);
        p.setBrush(Qt::green);
        p.drawRect(50, 50, 100, 100);
    }
public:
    Top(Bottom * bottom, QWidget * parent = 0) :
        QWidget(parent), m_bottom(bottom) {}
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;
    Bottom b;
    Top t(&b);
    w.setCentralWidget(&t);
    w.setMinimumSize(200, 200);
    w.show();
    return a.exec();
}
