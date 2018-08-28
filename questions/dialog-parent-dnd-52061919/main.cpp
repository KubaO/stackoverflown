// https://github.com/KubaO/stackoverflown/tree/master/questions/dialog-parent-dnd-52061919
#include <QtWidgets>

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);

   QWidget ui;
   QVBoxLayout layout{&ui};
   QPushButton button{"Toggle List"};
   QCheckBox workaround1{"Workaround 1"};
   QCheckBox workaround2{"Workaround 2"};
   for (auto w : QWidgetList{&button, &workaround1, &workaround2}) layout.addWidget(w);
   workaround2.setChecked(true);

   QListWidget listWidget;
   Q_ASSERT(!listWidget.testAttribute(Qt::WA_DeleteOnClose));
   listWidget.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   listWidget.setDragDropMode(QAbstractItemView::DragDrop);
   listWidget.setDefaultDropAction(Qt::MoveAction);
   for (auto s : QStringList{"first", "second", "third"}) listWidget.addItem(s);

   QObject::connect(&button, &QPushButton::pressed, [&] {
      if (!listWidget.parent()) {
         if (!workaround1.isChecked())
            listWidget.setParent(&button, listWidget.windowFlags());
         else {
            auto windowType =
                Qt::WindowType(int(listWidget.windowFlags() & Qt::WindowType_Mask));
            listWidget.setParent(&button,
                                 listWidget.windowFlags() & ~Qt::WindowType_Mask);
            listWidget.setWindowFlag(windowType);
         }
         listWidget.show();
      } else {
         if (!workaround2.isChecked())
            listWidget.setParent(nullptr);
         else
            listWidget.setParent(nullptr, listWidget.windowFlags());
         listWidget.close();
      }
   });

   ui.setMinimumSize(320, 200);
   ui.show();
   return app.exec();
}
