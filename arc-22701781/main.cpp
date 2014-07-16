#include <QApplication>
#include <QLabel>
#include <QPainter>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLabel label;
    QPixmap pix(256, 256);
    QPainter p(&pix);
    p.drawArc(0, 0, 256, 256, 0, -90*16);
    label.setPixmap(pix);
    label.show();
    return a.exec();
}
