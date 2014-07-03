#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QPushButton>

struct KMean {
    int getK() const { return 3; }
};

class Widget : public QPushButton
{
    Q_OBJECT
    KMean kmean;
    Q_SLOT void triggered(QAction* an) {
        const QVariant index(an->property("index"));
        if (!index.isValid()) return;
        const int i = index.toInt();
        setText(QString("Clicked %1").arg(i));
    }
    Q_SLOT void on_clicked() {
        QMenu * menu = new QMenu();
        int last = kmean.getK();
        for(int i = 1; i <= last; i++)
        {
            QAction * action = new QAction(QString("Merge with %1").arg(i), menu);
            action->setProperty("index", i);
            menu->addAction(action);
        }
        connect(menu, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
        menu->popup(mapToGlobal(rect().bottomRight()));
    }
public:
    Widget(QWidget *parent = 0) : QPushButton("Show Menu ...", parent) {
        connect(this, SIGNAL(clicked()), SLOT(on_clicked()));
    }
};

int main (int argc, char **argv)
{
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}

#include "main.moc"
