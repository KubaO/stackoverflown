// https://github.com/KubaO/stackoverflown/tree/master/questions/button-grid-22641306
#if 0

#include <QtGui>
#if QT_VERSION_MAJOR >= 5
#include <QtWidgets>
#endif

const char kIndex[] = "index";
struct Display : QLabel {
   Q_SLOT void onClicked() {
      setText(sender()->property(kIndex).toString());
   }
   Q_OBJECT
};

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   QWidget window;
   QGridLayout layout{&window};
   QVarLengthArray<QPushButton, 12> buttons(12);
   Display display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         auto index = QString{"(%1,%2)"}.arg(i).arg(j);
         auto & button = buttons[i*columns+j];
         button.setText(index);
         button.setProperty(kIndex, index);
         layout.addWidget(&button, i, j);
         display.connect(&button, SIGNAL(clicked()), SLOT(onClicked()));
      }
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}
#include "main.moc"

#endif

#if 0

#include <QtGui>
#if QT_VERSION_MAJOR >= 5
#include <QtWidgets>
#endif

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   QSignalMapper mapper;
   QWidget window;
   QGridLayout layout{&window};
   QVarLengthArray<QPushButton, 12> buttons(12);
   QLabel display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         auto text = QString{"(%1,%2)"}.arg(i).arg(j);
         auto & button = buttons[i*columns+j];
         button.setText(text);
         layout.addWidget(&button, i, j);
         mapper.connect(&button, SIGNAL(clicked()), SLOT(map()));
         mapper.setMapping(&button, text);
      }
   display.connect(&mapper, SIGNAL(mapped(QString)), SLOT(setText(QString)));
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}

#endif

#if 1

#include <QtWidgets>

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   QWidget window;
   QGridLayout layout{&window};
   QVarLengthArray<QPushButton, 12> buttons(12);
   QLabel display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         auto text = QStringLiteral("(%1,%2)").arg(i).arg(j);
         auto & button = buttons[i*columns+j];
         button.setText(text);
         layout.addWidget(&button, i, j);
         QObject::connect(&button, &QPushButton::clicked, [&display, text] {
            display.setText(text);
         });
      }
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}

#endif
