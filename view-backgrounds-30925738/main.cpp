#include <QApplication>
#include <QStringListModel>
#include <QTableView>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   auto data = QStringList() << "Foo" << "Baz" << "Bar" << "Car";
   QStringListModel model(data);
   QTableView view;

   view.setModel(&model);
   view.setEditTriggers(QAbstractItemView::NoEditTriggers);
   view.setSelectionMode(QAbstractItemView::MultiSelection);
   view.setSortingEnabled(true);

   app.setStyleSheet("QTableView { selection-background-color: yellow; selection-color: black; }");
   view.show();
   return app.exec();
}
