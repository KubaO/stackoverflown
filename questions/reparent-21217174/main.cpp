#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget * w1 = new QWidget, * w2 = new QWidget, * child = new QWidget;
    QTimer timer;
    w1->setLayout(new QGridLayout);
    w2->setLayout(new QGridLayout);
    w1->layout()->addWidget(child);
    QObject::connect(&timer, &QTimer::timeout, [=](){ w2->layout()->addWidget(child);});
    timer.start(1000);
    QTimer::singleShot(1500, qApp, "quit()");
    return a.exec();
}
