// https://github.com/KubaO/stackoverflown/tree/master/questions/polygon-sigslot-39528030
#include <QtWidgets>

class HoverFilter : public QGraphicsObject {
    Q_OBJECT
    bool sceneEventFilter(QGraphicsItem * watched, QEvent *event) override {
        if (event->type() == QEvent::GraphicsSceneHoverEnter)
            emit hoverEnter(watched);
        else if (event->type() == QEvent::GraphicsSceneHoverLeave)
            emit hoverLeave(watched);
        return false;
    }
    QRectF boundingRect() const override { return QRectF{}; }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
public:
    Q_SIGNAL void hoverEnter(QGraphicsItem *);
    Q_SIGNAL void hoverLeave(QGraphicsItem *);
};

QPolygonF equilateralTriangle(qreal size) {
    return QPolygonF{{{0.,0.}, {size/2., -size*sqrt(3.)/2.}, {size,0.}, {0.,0.}}};
}

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QWidget ui;
    QVBoxLayout layout{&ui};
    QGraphicsView view;
    QLabel label{"Hovering"};
    layout.addWidget(&view);
    layout.addWidget(&label);
    label.hide();
    ui.show();

    QGraphicsScene scene;
    view.setScene(&scene);
    HoverFilter filter;
    QGraphicsPolygonItem triangle{equilateralTriangle(100.)};
    scene.addItem(&filter);
    scene.addItem(&triangle);
    triangle.setAcceptHoverEvents(true);
    triangle.installSceneEventFilter(&filter);
    QObject::connect(&filter, &HoverFilter::hoverEnter, [&](QGraphicsItem * item) {
        if (item == &triangle) label.show();
    });
    QObject::connect(&filter, &HoverFilter::hoverLeave, [&](QGraphicsItem * item) {
        if (item == &triangle) label.hide();
    });
    return app.exec();
}

#include "main.moc"
