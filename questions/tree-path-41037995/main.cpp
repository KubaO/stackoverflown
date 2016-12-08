// https://github.com/KubaO/stackoverflown/tree/master/questions/tree-path-41037995
#include <QtWidgets>

QTreeWidgetItem *get(QTreeWidgetItem *parent, const QString &text) {
   for (int i = 0; i < parent->childCount(); ++i) {
      auto child = parent->child(i);
      if (child->text(0) == text)
         return child;
   }
   return new QTreeWidgetItem(parent, {text});
}

int main(int argc, char **argv)
{
   QApplication app(argc, argv);
   QStringList filenames{"TEST/branch", "TEST/foo", "trunk"};
   QWidget window;
   QVBoxLayout layout(&window);
   QTreeWidget treeWidget;
   QLabel label1, label2;

   for (const auto &filename : filenames) {
      QString path;
      auto item = treeWidget.invisibleRootItem();
      for (auto const &chunk : filename.split('/')) {
         item = get(item, chunk);
         path.append(QStringLiteral("/%1").arg(chunk));
         item->setData(0, Qt::UserRole, path);
      }
   }

   QObject::connect(&treeWidget, &QTreeWidget::currentItemChanged, [&](const QTreeWidgetItem *item){
      QString path;
      for (; item; item = item->parent())
         path.prepend(QStringLiteral("/%1").arg(item->text(0)));
      label1.setText(path);
   });

   QObject::connect(&treeWidget, &QTreeWidget::currentItemChanged, [&](const QTreeWidgetItem *item){
      label2.setText(item->data(0, Qt::UserRole).toString());
   });

   layout.addWidget(&treeWidget);
   layout.addWidget(&label1);
   layout.addWidget(&label2);
   window.show();
   return app.exec();
}
