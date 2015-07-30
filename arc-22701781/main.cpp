#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QPicture>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    QLabel label;
    QPicture pic;
    pic.setBoundingRect(QRect(0, 0, 100, 100));
    QPainter p(&pic);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawArc(0, 0, 100, 100, 0, -90*16);
    p.end();
    label.setPicture(pic);
    label.show();
    return a.exec();
}
