#if 0
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

class Display : public QLabel {
   Q_OBJECT
public:
   Q_SLOT void onClicked() {
      setText(sender()->property("index").toString());
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget window;
   QGridLayout layout(&window);
   Display display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         QString index = QStringLiteral("(%1,%2)").arg(i).arg(j);
         QPushButton * btn = new QPushButton(index);
         btn->setProperty("index", index);
         layout.addWidget(btn, i, j);
         display.connect(btn, SIGNAL(clicked()), SLOT(onClicked()));
      }
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}

#include "main.moc"

#endif

#if 1

#include <QApplication>
#include <QSignalMapper>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QSignalMapper mapper;
   QWidget window;
   QGridLayout layout(&window);
   QLabel display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         QString index = QStringLiteral("(%1,%2)").arg(i).arg(j);
         QPushButton * btn = new QPushButton(index);
         layout.addWidget(btn, i, j);
         mapper.connect(btn, SIGNAL(clicked()), SLOT(map()));
         mapper.setMapping(btn, index);
      }
   display.connect(&mapper, SIGNAL(mapped(QString)), SLOT(setText(QString)));
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}

#endif

#if 0

#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget window;
   QGridLayout layout(&window);
   QLabel display;

   const int rows = 4, columns = 3;
   for (int i = 0; i < rows; ++ i)
      for (int j = 0; j < columns; ++j) {
         QString index = QStringLiteral("(%1,%2)").arg(i).arg(j);
         QPushButton * btn = new QPushButton(index);
         layout.addWidget(btn, i, j);
         QObject::connect(btn, &QPushButton::clicked, [&display, index] {
            display.setText(index);
         });
      }
   layout.addWidget(&display, rows, 0, 1, columns);

   window.show();
   return a.exec();
}

#endif
