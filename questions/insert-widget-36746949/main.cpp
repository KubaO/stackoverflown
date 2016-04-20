// https://github.com/KubaO/stackoverflown/tree/master/questions/insert-widget-36746949
#include <QtWidgets>

namespace SO { enum InsertPosition { InsertBefore, InsertAfter }; }

bool insertWidget(QBoxLayout * layout, QWidget * reference, QWidget * widget,
                  SO::InsertPosition pos = SO::InsertBefore, int stretch = 0,
                  Qt::Alignment alignment = 0) {
   int index = -1;
   for (int i = 0; i < layout->count(); ++i)
      if (layout->itemAt(i)->widget() == reference) {
         index = i;
         break;
      }
   if (index < 0) return false;
   if (pos == SO::InsertAfter) index++;
   layout->insertWidget(index, widget, stretch, alignment);
   return true;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QVBoxLayout l{&w};
   QLabel first{"First"};
   QLabel second{"Second"};
   l.addWidget(&first);
   l.addWidget(&second);
   insertWidget(&l, &first, new QLabel{"Before First"}, SO::InsertBefore);
   insertWidget(&l, &second, new QLabel{"After Second"}, SO::InsertAfter);
   w.show();
   return app.exec();
}

