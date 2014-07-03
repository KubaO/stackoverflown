#include <QGuiApplication>
#include <QGraphicsScene>
#include <QTimer>

int main(int argc, char ** argv) {
    QGuiApplication app(argc, argv);
    QGraphicsScene scene;
    QTimer::singleShot(1000, &app, SLOT(quit()));
    return app.exec();
}
