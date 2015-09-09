// https://github.com/KubaO/stackoverflown/tree/master/questions/tablewidget-32403753
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};

   QWidget w;
   QVBoxLayout layout { &w };

   QTableWidget tw;
   tw.setShowGrid(true);
   tw.setColumnCount(2);
   auto header = QStringList() << "Header1" << "Header2";
   tw.setHorizontalHeaderLabels(header);
   tw.setSortingEnabled(true);

   QPushButton button { "Add Row" };
   QObject::connect(&button, &QPushButton::clicked, &tw, [&tw]{
      auto row = tw.rowCount();
      tw.insertRow(row);

      auto item1 = new QTableWidgetItem { "Cell1" };
      item1->setBackground(Qt::red);
      tw.setItem(row, 0, item1);
      row = item1->row(); // Needed to support sorted tables.

      auto item2 = new QTableWidgetItem { "Cell2" };
      item2->setBackground(Qt::green);
      tw.setItem(row, 1, item2);
   });

   layout.addWidget(&tw);
   layout.addWidget(&button);
   w.show();
   return app.exec();
}
