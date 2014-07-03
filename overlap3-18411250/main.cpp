#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

qreal rnd() { return qrand() / (float)RAND_MAX; }

class OverlaidGraphicsView : public QGraphicsView
{
    Q_OBJECT
    QGraphicsScene * m_overlayScene;
public:
    explicit OverlaidGraphicsView(QWidget* parent = 0) :
        QGraphicsView(parent), m_overlayScene(NULL) {}
    explicit OverlaidGraphicsView(QGraphicsScene * scene = 0, QWidget * parent = 0) :
        QGraphicsView(scene, parent), m_overlayScene(NULL) {}
    void setOverlayScene(QGraphicsScene * scene) {
        if (scene == m_overlayScene) return;
        m_overlayScene = scene;
        connect(scene, SIGNAL(changed(QList<QRectF>)), SLOT(overlayChanged()));
        update();
    }
    QGraphicsScene * overlayScene() const { return m_overlayScene; }
    void paintEvent(QPaintEvent *ev) {
        QGraphicsView::paintEvent(ev);
        if (m_overlayScene) paintOverlay();
    }
    virtual void paintOverlay() {
        QPainter p(viewport());
        p.setRenderHints(renderHints());
        m_overlayScene->render(&p, viewport()->rect());
    }
    Q_SLOT void overlayChanged() { update(); }
};

class Window : public QWidget
{
    QGraphicsScene scene, notification;
    OverlaidGraphicsView * view;
    QGraphicsSimpleTextItem * item;
    int timerId;
    int time;
public:
    Window() :
        view(new OverlaidGraphicsView(&scene, this)),
        timerId(-1), time(0)
    {
        for (int i = 0; i < 20; ++ i) {
            qreal w = rnd()*0.3, h = rnd()*0.3;
            scene.addEllipse(rnd()*(1-w), rnd()*(1-h), w, h, QPen(Qt::red), QBrush(Qt::lightGray));
        }
        view->fitInView(0, 0, 1, 1);
        view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
        view->setRenderHint(QPainter::Antialiasing);
        view->setOverlayScene(&notification);
        item = new QGraphicsSimpleTextItem();
        item->setPen(QPen(Qt::blue));
        item->setBrush(Qt::NoBrush);
        item->setPos(95, 0);
        notification.addItem(item);
        notification.addRect(0, 0, 100, 0, Qt::NoPen, Qt::NoBrush); // strut
        timerId = startTimer(1000);
        QTimerEvent ev(timerId);
        timerEvent(&ev);
    }
    void resizeEvent(QResizeEvent * ev) {
        view->resize(size());
        view->fitInView(0, 0, 1, 1, Qt::KeepAspectRatio);
        QWidget::resizeEvent(ev);
    }
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() != timerId) return;
        item->setText(QString::number(time++));
    }
};

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);

    Window window;
    window.show();
    a.exec();
}

#include "main.moc"
