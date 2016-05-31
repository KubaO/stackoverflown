// https://github.com/KubaO/stackoverflown/tree/master/questions/button-grid-37492290
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
const QString numerals[] = {"zero", "one", "two", "three", "four", "five", "six", "seven",
                            "eight", "nine", "ten"};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QGridLayout layout{&w};
   QLabel label;
   QSignalMapper mapper;
   QPushButton buttons[10];
   for (int i = 0; i < 10; ++i) {
      auto n = qMax(7-(3*(i/3))+i%3, 0); // numpad layout
      auto & button = buttons[i];
      button.setText(QString::number(n));
      mapper.setMapping(&button, QString("%1 - %2").arg(n).arg(numerals[n]));
      mapper.connect(&button, SIGNAL(clicked(bool)), SLOT(map()));
      layout.addWidget(&button, 1+i/3, i%3, 1, n > 0 ? 1 : 3);
   }
   layout.addWidget(&label, 0, 0, 1, 3);
   label.connect(&mapper, SIGNAL(mapped(QString)), SLOT(setText(QString)));
   w.show();
   return app.exec();
}
