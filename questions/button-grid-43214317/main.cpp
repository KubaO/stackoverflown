// https://github.com/KubaO/stackoverflown/tree/master/questions/button-grid-43214317
#include <QtWidgets>

namespace Ui { class Window {
public:
   // Approximate uic output
   QGridLayout *layout;
   void setupUi(QWidget * widget) {
      layout = new QGridLayout(widget);
   }
}; }

class Window : public QWidget
{
   Q_OBJECT
   Ui::Window ui;
   QPushButton * buttonAt(int row, int column) {
      auto item = ui.layout->itemAtPosition(row, column);
      return item ? qobject_cast<QPushButton*>(item->widget()) : nullptr;
   }

public:
   explicit Window(QWidget *parent = {});
};

Window::Window(QWidget *parent) : QWidget(parent) {
   ui.setupUi(this);
   for (int i = 0; i < 5; ++i)
      for (int j = 0; j < 6; ++j)
      {
         auto b = new QPushButton(QStringLiteral("%1,%2").arg(i).arg(j));
         ui.layout->addWidget(b, i, j);
      }
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Window w;
   w.show();
   return a.exec();
}
#include "main.moc"
