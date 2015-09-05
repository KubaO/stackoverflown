#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QVBoxLayout layout{&w};
   // Individual Buttons
   QPushButton p1{"button1"}, p2{"button2"};
   for (auto p : {&p1, &p2}) {
      layout.addWidget(p);
      p->setFocusPolicy(Qt::StrongFocus);
   }
   // A button box
   QDialogButtonBox box;
   for (auto text : {"button3", "button4"})
      box.addButton(text, QDialogButtonBox::NoRole)->setFocusPolicy(Qt::StrongFocus);
   layout.addWidget(&box);

   w.show();
   return app.exec();
}

