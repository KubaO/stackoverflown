#include <QApplication>
#include <QListView>
#include <QStringListModel>
#include <QThread>

class Loader : public QObject {
    Q_OBJECT
public:
    Loader(QObject *parent = 0) : QObject(parent) {}
    Q_SLOT void load() {
        // this could be reading from a file
        QStringList lines;
        for (int i = 0; i < 100000; ++i) {
            lines << QString("Item %1").arg(i);
        }
        emit hasData(lines);
    }
    Q_SIGNAL void hasData(const QStringList &);
};

class StringListModel : public QStringListModel {
    Q_OBJECT
public:
    StringListModel() {}
    Q_SLOT void setList(const QStringList & list) { setStringList(list); }
};

int main(int argc, char *argv[])
{
    QThread thread;
    StringListModel model;
    QApplication a(argc, argv);
    QListView view;
    Loader loader;
    view.setModel(&model);
    view.setUniformItemSizes(true);
    loader.moveToThread(&thread);
    loader.connect(&thread, SIGNAL(started()), SLOT(load()));
    model.connect(&loader, SIGNAL(hasData(QStringList)), SLOT(setList(QStringList)));
    thread.connect(qApp, SIGNAL(aboutToQuit()), SLOT(quit()));
    thread.start();
    view.show();
    return a.exec();
}

#include "main.moc"
