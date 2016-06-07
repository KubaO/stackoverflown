// https://github.com/KubaO/stackoverflown/tree/master/questions/camlist-37603946
#include <QtWidgets>
#include <QtMultimedia>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QComboBox combo;
   QObject::connect(&combo, &QComboBox::currentTextChanged, [&]{
      qDebug() << combo.currentText();
   });
   for (auto const & info : QCameraInfo::availableCameras())
      combo.addItem(info.description());
   combo.show();
   return app.exec();
}
