#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QQuickWindow>
#include <QtQml>
#include <QDebug>

class Settings : public QSettings
{
    Q_OBJECT
    public:
    Settings() : QSettings("Marcin Mielniczuk", "BigText") {}
    ~Settings() { qDebug() << "Dying"; }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<Settings>("BigText", 1, 0, "Settings");
    engine.load(QUrl("qrc:/main.qml"));
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show();
    return app.exec();
}

#include "main.moc"
