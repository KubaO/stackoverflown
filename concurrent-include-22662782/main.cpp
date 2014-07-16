#include <QCoreApplication>
#include <QException>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QException e;
    return a.exec();
}
