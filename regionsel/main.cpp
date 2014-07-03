#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGLWidget>

static qreal rnd(qreal max) { return (qrand() / static_cast<qreal>(RAND_MAX)) * max; }

class View : public QGraphicsView {
public:
    View(QGraphicsScene *scene, QWidget *parent = 0) : QGraphicsView(scene, parent) {
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    }
    void drawBackground(QPainter *, const QRectF &) {
        QColor bg(Qt::blue);
        glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};

void setupScene(QGraphicsScene &s)
{
    for (int i = 0; i < 10; i++) {
        qreal x = rnd(1), y = rnd(1);
        QAbstractGraphicsShapeItem * item = new QGraphicsRectItem(x, y, rnd(1-x), rnd(1-y));
        item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        item->setPen(QPen(Qt::red, 0));
        item->setBrush(Qt::lightGray);
        s.addItem(item);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene s;
    setupScene(s);
    View v(&s);
    v.fitInView(0, 0, 1, 1);
    v.show();
    v.setDragMode(QGraphicsView::RubberBandDrag);
    v.setRenderHint(QPainter::Antialiasing);
    return a.exec();
}
