#include <QCoreApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   qDebug() << QCoreApplication::applicationDirPath();
   QFile data(QCoreApplication::applicationDirPath() + "/../Resources/data.txt");
   if (data.open(QIODevice::ReadOnly | QIODevice::Text))
      qDebug() << data.readAll();
   return 0;
}
