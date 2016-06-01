// https://github.com/KubaO/stackoverflown/tree/master/questions/dummymodel-37577922
#include <QtWidgets>

template <typename T> QStandardItem * newItem(const T val) {
  auto item = new QStandardItem;
  item->setData(val, Qt::DisplayRole);
  return item;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};

   QStandardItemModel model;
   model.setColumnCount(3);
   model.setHorizontalHeaderLabels(QStringList{"Error Number", "Message", "Details"});

   auto newRow = []{ return QList<QStandardItem*>{
         newItem(1),
         newItem("Unable to perform snapshot"),
         newItem("Unable to perform snapshot. Please try again")};
   };
   model.appendRow(newRow());
   model.appendRow(newRow());

   QTableView view;
   view.setModel(&model);
   view.show();

   return app.exec();
}
