#include <QtWidgets>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QComboBox cb;
   cb.addItem("Foo");
   cb.insertSeparator(1);
   cb.addItem("Bar");
   Q_ASSERT(cb.count() == 3);
   return 0;
}
