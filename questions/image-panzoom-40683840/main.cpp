// https://github.com/KubaO/stackoverflown/tree/master/questions/image-panzoom-40683840
#include <QtWidgets>
#include <QtNetwork>

class ImageViewer : public QWidget {
    QPixmap m_pixmap;
    QRectF m_rect;
    QPointF m_reference;
    QPointF m_delta;
    qreal m_scale = 1.0;
    void paintEvent(QPaintEvent *) override {
        QPainter p{this};
        p.translate(rect().center());
        p.scale(m_scale, m_scale);
        p.translate(m_delta);
        p.drawPixmap(m_rect.topLeft(), m_pixmap);
    }
    void mousePressEvent(QMouseEvent *event) override {
        m_reference = event->pos();
        qApp->setOverrideCursor(Qt::ClosedHandCursor);
        setMouseTracking(true);
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        m_delta += (event->pos() - m_reference) * 1.0/m_scale;
        m_reference = event->pos();
        update();
    }
    void mouseReleaseEvent(QMouseEvent *) override {
        qApp->restoreOverrideCursor();
        setMouseTracking(false);
    }
public:
    void setPixmap(const QPixmap &pix) {
        m_pixmap = pix;
        m_rect = m_pixmap.rect();
        m_rect.translate(-m_rect.center());
        update();
    }
    void scale(qreal s) {
        m_scale *= s;
        update();
    }
    QSize sizeHint() const override { return {400, 400}; }
};

class SceneImageViewer : public QGraphicsView {
    QGraphicsScene m_scene;
    QGraphicsPixmapItem m_item;
public:
    SceneImageViewer() {
        setScene(&m_scene);
        m_scene.addItem(&m_item);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }
    void setPixmap(const QPixmap &pixmap) {
        m_item.setPixmap(pixmap);
        auto offset = -QRectF(pixmap.rect()).center();
        m_item.setOffset(offset);
        setSceneRect(offset.x()*4, offset.y()*4, -offset.x()*8, -offset.y()*8);
        translate(1, 1);
    }
    void scale(qreal s) { QGraphicsView::scale(s, s); }
    QSize sizeHint() const override { return {400, 400}; }
};

int main(int argc, char *argv[])
{
    QApplication a{argc, argv};
    QWidget ui;
    QGridLayout layout{&ui};
    ImageViewer viewer1;
    SceneImageViewer viewer2;
    QPushButton zoomOut{"Zoom Out"}, zoomIn{"Zoom In"};
    layout.addWidget(&viewer1, 0, 0);
    layout.addWidget(&viewer2, 0, 1);
    layout.addWidget(&zoomOut, 1, 0, 1, 1, Qt::AlignLeft);
    layout.addWidget(&zoomIn, 1, 1, 1, 1, Qt::AlignRight);

    QNetworkAccessManager mgr;
    QScopedPointer<QNetworkReply> rsp(
                mgr.get(QNetworkRequest({"http://i.imgur.com/ikwUmUV.jpg"})));
    QObject::connect(rsp.data(), &QNetworkReply::finished, [&]{
        if (rsp->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            pixmap.loadFromData(rsp->readAll());
            viewer1.setPixmap(pixmap);
            viewer2.setPixmap(pixmap);
        }
        rsp.reset();
    });
    QObject::connect(&zoomIn, &QPushButton::clicked, [&]{
        viewer1.scale(1.1); viewer2.scale(1.1);
    });
    QObject::connect(&zoomOut, &QPushButton::clicked, [&]{
        viewer1.scale(1.0/1.1); viewer2.scale(1.0/1.1);
    });
    ui.show();
    return a.exec();
}
