#include <QApplication>
#include <QLabel>

class Window : public QLabel {
    Q_OBJECT
public:
    Q_SIGNAL void closing();
    void closeEvent(QCloseEvent*) {
        emit closing();
    }
    explicit Window(const QString & str) : QLabel(str) {
        setMinimumSize(100, 100);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w1("One"), w2("Two");
    QObject::connect(&w1, SIGNAL(closing()), &w2, SLOT(close()));
    QObject::connect(&w2, SIGNAL(closing()), &w1, SLOT(close()));
    w1.show();
    w2.show();
    return a.exec();
}

#include "main.moc"
