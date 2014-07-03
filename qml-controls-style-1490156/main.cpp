    #include <QGuiApplication>
    #include <QQmlApplicationEngine>
    #include <QQuickWindow>
    #include <QImage>
    #include <QPainter>
    #include <QQuickImageProvider>
    #include <QDebug>

    class ImageProvider : public QQuickImageProvider
    {
    public:
        ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
        QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
            QImage img(32, 32, QImage::Format_ARGB32_Premultiplied);
            img.fill(0); // transparent
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.translate(16, 16);
            p.scale(14, 14);
            p.setPen(QPen(Qt::black, 0.1));
            if (id == "img1") {
                p.drawEllipse(QPointF(0, 0), 1, 1);
            }
            else if (id == "img2") {
                p.drawLine(-1, -1, 1, 1);
                p.drawLine(-1, 1, 1, -1);
            }
            *size = img.size();
            return img;
        }
    };

    int main(int argc, char *argv[])
    {
        QGuiApplication app(argc, argv);
        QQmlApplicationEngine engine;
        engine.addImageProvider("images", new ImageProvider);
        engine.load(QUrl("qrc:///main.qml"));
        QObject *topLevel = engine.rootObjects().value(0);
        QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
        window->show();
        return app.exec();
    }
