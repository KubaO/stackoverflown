#include <QApplication>
#include "gui.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Gui w;
   w.show();
   return a.exec();
}

