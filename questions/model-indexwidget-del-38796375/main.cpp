// https://github.com/KubaO/stackoverflown/tree/master/questions/model-indexwidget-del-38796375
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QSet<QObject*> live;
   {
      QDialog dialog;
      QVBoxLayout layout{&dialog};
      QTableView view;
      QPushButton clear{"Clear"};
      layout.addWidget(&view);
      layout.addWidget(&clear);

      QScopedPointer<QStringListModel> model{new QStringListModel{&dialog}};
      model->setStringList(QStringList{"a", "b", "c"});
      view.setModel(model.data());
      for (int i = 0; i < model->rowCount(); ++i) {
         auto deleteButton = new QPushButton;
         view.setIndexWidget(model->index(i), deleteButton);
         live.insert(deleteButton);
         QObject::connect(deleteButton, &QObject::destroyed, [&](QObject* obj) {
            live.remove(obj); });
      }
      QObject::connect(&clear, &QPushButton::clicked, [&]{ model.reset(); });
      dialog.exec();
      Q_ASSERT(model || live.isEmpty());
   }
   Q_ASSERT(live.isEmpty());
}
