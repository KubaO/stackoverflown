#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLabel label("foobar");
    int number = 100;
    QString style = QString("QLabel {background-color: rgb(%1, %1, %1) }")
                    .arg(number);
    label.setStyleSheet(style);
    label.show();
    return a.exec();
}
