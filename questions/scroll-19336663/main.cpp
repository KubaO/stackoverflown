#include <QApplication>
#include <QScrollArea>
#include <QLabel>
#include <QScopedPointer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QScopedPointer<QScrollArea> area (new QScrollArea);
    QLabel * label = new QLabel(QString(300, 'm'));
    area->setWidget(label);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->show();
    return a.exec();
}
