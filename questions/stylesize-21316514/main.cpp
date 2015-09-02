#include <QApplication>
#include <QLabel>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString style("QLabel { min-height:100px; max-height:100px; min-width: 300px; max-width:300px }");
    a.setStyleSheet(style);
    QLabel l1, l2, l3;
    l1.ensurePolished(); // first method
    QCoreApplication::sendPostedEvents(&l2, 0); // second method
    l3.setText(QString("l1: %1 x %2 l2: %3 x %4 l3: %5 x %6")
               .arg(l1.width()).arg(l1.height())
               .arg(l2.width()).arg(l2.height())
               .arg(l3.width()).arg(l3.height()));
    l3.setAlignment(Qt::AlignCenter);
    l3.show();
    return a.exec();
}
