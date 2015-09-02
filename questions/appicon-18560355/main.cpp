#include <QFontDialog>
#include <QApplication>
#include <QIcon>
#include <QPainter>

class Dialog : public QFontDialog {
public:
    Dialog(QWidget *parent = 0) : QFontDialog(parent) {}
    Dialog(const QFont & initial, QWidget *parent = 0) : QFontDialog(initial, parent) {}
};

QIcon myIcon(const QColor & color)
{
    QIcon icon;
    QPixmap pm(128, 128);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.translate(64, 64);
    p.scale(50, 50);
    p.setBrush(color);
    p.setPen(QPen(Qt::lightGray, 0.1));
    p.drawEllipse(-1, -1, 2, 2);
    icon.addPixmap(pm);
    return icon;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(myIcon(Qt::red));
    Dialog d;
    d.setWindowIcon(myIcon(Qt::blue));
    d.show();
    return a.exec();
}
