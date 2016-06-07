// https://github.com/KubaO/stackoverflown/tree/master/questions/tostdstr-37669003
#include <QtWidgets>
#include <iostream>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QComboBox combo;
   for (auto & item : QStringList{"foo", "bar", "baz"})
      combo.addItem(item);
   QObject::connect(&combo, &QComboBox::currentTextChanged, [&]{
      std::cout << combo.currentText().toStdString() << std::endl;
   });
   combo.show();
   return app.exec();
}
