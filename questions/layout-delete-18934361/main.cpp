#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

static void clearLayout(QLayout * layout) {

}

class Widget : public QWidget {
    Q_OBJECT
    int m_counter;
public:
    Widget(QWidget * parent = 0) : QWidget(parent), m_counter(0) {
        new QVBoxLayout(this);
        newChildren();
    }

    Q_SLOT void newChildren() {
        foreach(QObject * w, this->children()) {
            if (w->isWidgetType()) delete w;
        }
        const int N = 1 + qrand() % 10;
        for (int i = 0; i < N; ++ i) {
            QPushButton * btn = new QPushButton(QString("Button #%1").arg(m_counter++));
            this->layout()->addWidget(btn);
            connect(btn, SIGNAL(clicked()), SLOT(newChildren()));
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

#include "main.moc"
