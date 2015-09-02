    #include <QApplication>
    #include <QGraphicsScene>
    #include <QGraphicsView>
    #include <QGraphicsItem>
    #include <QPainter>

    class Item : public QGraphicsItem
    {
        QBrush m_brush;
    public:
        explicit Item(bool nested = true, QGraphicsItem* parent = 0) : QGraphicsItem(parent), m_brush(Qt::white)
        {
            if (nested) {
                Item* item = new Item(false, this);
                item->setPos(10,10);
                m_brush = Qt::red;
            }
            setAcceptHoverEvents(true);
        }
        QRectF boundingRect() const
        {
            return QRectF(0,0,100,100);
        }
        void hoverEnterEvent(QGraphicsSceneHoverEvent *)
        {
            m_brush = Qt::red;
            update();
        }
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
        {
            m_brush = Qt::white;
            update();
        }
        void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
        {
            p->setBrush(m_brush);
            p->drawRoundRect(boundingRect());
        }
    };

    int main(int argc, char *argv[])
    {
        QApplication app(argc, argv);
        QGraphicsScene scene;
        scene.addItem(new Item);
        QGraphicsView view;
        view.setScene(&scene);
        view.setMouseTracking(true);
        view.show();
        return app.exec();
    }
