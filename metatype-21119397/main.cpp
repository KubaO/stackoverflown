#include <QCoreApplication>
#include <QDebug>

namespace NS {
    struct MyType
    {
        int val;
        MyType() {}
        MyType(int v) : val(v) {}
    };

    class Object : public QObject {
        Q_OBJECT
    public:
        Q_SIGNAL void goodSignal(const NS::MyType &);
        Q_SIGNAL void badSignal(const MyType &);
        Q_SLOT void slot(const NS::MyType & x) {
            qDebug() << "Successful slot call" << x.val;
            qApp->quit();
        }
    };
}
Q_DECLARE_METATYPE(NS::MyType)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NS::Object src, dst;
    qRegisterMetaType<NS::MyType>();

    qDebug() << "Now we fail";
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QObject::connect(&src, &NS::Object::badSignal, &dst, &NS::Object::slot, Qt::QueuedConnection);
#else
    dst.connect(&src, SIGNAL(badSignal(NS::MyType)), SLOT(slot(NS::MyType)), Qt::QueuedConnection);
#endif
    emit src.goodSignal(NS::MyType(1));

    qDebug() << "Now we succeed";
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QObject::connect(&src, &NS::Object::goodSignal, &dst, &NS::Object::slot, Qt::QueuedConnection);
#else
    dst.connect(&src, SIGNAL(goodSignal(NS::MyType)), SLOT(slot(NS::MyType)), Qt::QueuedConnection);
#endif
    emit src.goodSignal(NS::MyType(1));

    return a.exec();
}

#include "main.moc"
