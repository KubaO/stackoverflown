// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-movable-circles-11188261
//main.cpp
#include <cmath>
#include <QGraphicsView>
#include <QPushButton>
#include <QGridLayout>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QApplication>

class Circle : public QGraphicsEllipseItem
{
    QBrush brush;
public:
    Circle(const QPointF & c) : brush("lightgray") {
        const qreal r = 10.0 + (50.0*qrand())/RAND_MAX;
        setRect(QRectF(c.x()-r, c.y()-r, 2.0*r, 2.0*r));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsFocusable);
        setPen(QPen("red"));
        setBrush(brush);
    }
    void focusInEvent(QFocusEvent *) { setBrush(QBrush("red")); }
    void focusOutEvent(QFocusEvent *) { setBrush(brush); }
};

class Painter : public QWidget
{
    Q_OBJECT
    QGraphicsView * view;
    QGraphicsScene * scene;
public:
    explicit Painter(QWidget *parent = 0);
protected slots:
    void on_remove_clicked();
protected:
   void mousePressEvent(QMouseEvent *event);
};

Painter::Painter(QWidget *parent) :
    QWidget(parent),
    view(new QGraphicsView(this)),
    scene(new QGraphicsScene(this))
{
    QGridLayout * layout = new QGridLayout();
    layout->addWidget(view, 0, 0, 1, 2);
    QPushButton * button = new QPushButton("Clear", this);
    connect(button, SIGNAL(clicked()), scene, SLOT(clear()));
    layout->addWidget(button, 1, 0);
    button = new QPushButton("Remove", this);
    button->setObjectName("remove");
    layout->addWidget(button, 1, 1);
    setLayout(layout);
    QMetaObject::connectSlotsByName(this);

    view->setRenderHint(QPainter::Antialiasing);
    view->setScene(scene);
    view->setSceneRect(0,0,500,500);
}

void Painter::mousePressEvent(QMouseEvent *event)
{
    const QPointF center = view->mapToScene(view->mapFromParent(event->pos()));
    scene->addItem(new Circle(center));
}

void Painter::on_remove_clicked()
{
    delete scene->focusItem();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Painter p;
    p.show();
    return a.exec();
}

#include "main.moc"
