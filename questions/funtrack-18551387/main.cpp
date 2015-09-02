#include <QApplication>
#include <QMouseEvent>
#include <QTransform>
#include <QLabel>

class Window : public QLabel {
public:
    Window(QWidget *parent = 0, Qt::WindowFlags f = 0) : QLabel(parent, f) {
        setMouseTracking(true);
        setMinimumSize(100, 100);
    }
    void mouseMoveEvent(QMouseEvent *ev) {
        QTransform t;
        t.scale(1, -1);
        t.translate(0, -height()+1);
        QPoint pos = ev->pos() * t;
        setText(QString("%1, %2").arg(pos.x()).arg(pos.y()));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}
