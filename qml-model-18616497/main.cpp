#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QStringListModel>
#include <QQmlContext>

class Generator : public QObject
{
    Q_OBJECT
    QStringListModel * m_model;
public:
    Generator(QStringListModel * model) : m_model(model) {}
    Q_INVOKABLE void generate(const QVariant & val) {
        QStringList list;
        for (int i = 1; i <= 3; ++i) {
            list << QString("%1:%2").arg(val.toString()).arg(i);
        }
        m_model->setStringList(list);
    }
};

int main(int argc, char *argv[])
{
    QStringListModel model1, model2;
    Generator generator(&model2);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QStringList list;
    list << "one" << "two" << "three" << "four";
    model1.setStringList(list);

    engine.rootContext()->setContextProperty("model1", &model1);
    engine.rootContext()->setContextProperty("model2", &model2);
    engine.rootContext()->setContextProperty("generator", &generator);

    engine.load(QUrl("qrc:/main.qml"));
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show();
    return app.exec();
}

#include "main.moc"
