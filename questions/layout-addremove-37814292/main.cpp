// https://github.com/KubaO/stackoverflown/tree/master/questions/layout-addremove-37814292
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   QWidget widget;
   QVBoxLayout layout(&widget);
   QPushButton button;
   QLabel label("Hello");
   layout.addWidget(&button);
   layout.addWidget(&label);

   auto onClick = [&]{
      if (layout.count() == 3) {
         delete layout.itemAt(2)->widget();
         button.setText("Add");
      } else {
         layout.addWidget(new QLabel("Hello too!"));
         button.setText("Remove");
      }
   };
   QObject::connect(&button, &QPushButton::clicked, onClick);
   onClick();

   widget.show();
   return app.exec();
}
