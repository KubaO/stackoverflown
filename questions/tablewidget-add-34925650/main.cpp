// https://github.com/KubaO/stackoverflown/tree/master/questions/tablewidget-add-34925650
#include <QtWidgets>

int main(int argc, char ** argv) {
   typedef QObject Q;
   QApplication app{argc, argv};

   QWidget w;
   QVBoxLayout layout{&w};
   QTableWidget table;
   QLabel message1, message2;
   QPushButton button{"Add Item"};
   layout.addWidget(&table);
   layout.addWidget(&message1);
   layout.addWidget(&message2);
   layout.addWidget(&button);
   w.show();

   table.setColumnCount(1);
   Q::connect(&button, &QPushButton::clicked, &table, [&table]{
      auto r = table.rowCount();
      auto item = new QTableWidgetItem(QStringLiteral("Item %1").arg(r+1));
      table.insertRow(r);
      table.setItem(r, 0, item);
   });
   Q::connect(table.model(), &QAbstractItemModel::rowsInserted, &message1,
              [&](const QModelIndex &, int first, int last){
      message1.setText(QStringLiteral("Rows inserted %1:%2").arg(first).arg(last));
   });
   Q::connect(table.model(), &QAbstractItemModel::dataChanged, &message2,
              [&](const QModelIndex & topLeft, const QModelIndex &, const QVector<int>&){
      message2.setText(QStringLiteral("New data: \"%1\"").arg(topLeft.data().toString()));
   });

   return app.exec();
}
