#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    QHBoxLayout * l = new QHBoxLayout(&w);
    QPushButton * b = new QPushButton("Default");
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    l->addWidget(b);
    b = new QPushButton("Styled");
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    b->setStyleSheet(
                "* { border: 2px solid #8f8f91; border-radius: 12px; background-color: #d02020; }"
                "*:pressed { background-color: #f6f7fa; }");
    l->addWidget(b);
    QPalette pal = w.palette();
    pal.setBrush(QPalette::Background, Qt::darkBlue);
    w.setPalette(pal);
    w.show();
    return a.exec();
}

