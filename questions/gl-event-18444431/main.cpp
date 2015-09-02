//main.cpp
#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QtOpenGL/QGLWidget>

class TestView : public QGraphicsView
{
public:
    explicit TestView(QWidget* parent = 0) : QGraphicsView(parent) {
        setViewport(new QGLWidget());
        setScene(new QGraphicsScene(this));
        scene()->addEllipse(-50, -50, 100, 100, QPen(Qt::red), QBrush(Qt::lightGray));
    }
};

class Pane : public QWidget
{
public:
    explicit Pane(bool hasView, const QCursor & cur, QWidget *parent = 0) :
        QWidget(parent)
    {
        QVBoxLayout * l = new QVBoxLayout(this);
        QPushButton * btn = new QPushButton("[Pane]");
        btn->setCursor(cur);
        l->addWidget(btn);
        if (hasView) l->addWidget(new TestView()); else l->addStretch();
    }
};

class MainWindow : public QWidget
{
    Q_OBJECT
    QStackedWidget *sw;
public:
    explicit MainWindow(QWidget *parent = 0) : QWidget(parent) {
        QVBoxLayout *l = new QVBoxLayout(this);
        QPushButton *btn = new QPushButton("[Main Window] Flip Pages");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, SIGNAL(clicked()), SLOT(nextPage()));
        sw = new QStackedWidget();
        l->addWidget(btn);
        l->addWidget(sw);
        sw->addWidget(new Pane(true, Qt::OpenHandCursor));
        sw->addWidget(new Pane(false, Qt::ClosedHandCursor));
    }
    Q_SLOT void nextPage() { sw->setCurrentIndex((sw->currentIndex() + 1) % sw->count()); }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "main.moc"
