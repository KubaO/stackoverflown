// https://github.com/KubaO/stackoverflown/tree/master/questions/layout-take-40497358
#include <QtWidgets>
#include "ui_form.h"

QLayout * takeLayout(QWidget *);

int main(int argc, char ** argv)
{
   QApplication app{argc, argv};

   QWidget parent;
   QHBoxLayout layout{&parent};
   Ui::Form ui;
   {
      QWidget w;
      ui.setupUi(&w);
      w.dumpObjectTree();
      if (true) {
         ui.layout->setParent(nullptr);
         delete ui.wrapper;
         layout.addLayout(ui.layout);
      } else {
         layout.addLayout(takeLayout(&w));
      }
   }
   parent.show();
   return app.exec();
}

#include <private/qwidget_p.h>
#include <private/qlayout_p.h>

class WidgetHelper : private QWidget {
   struct LayoutHelper : private QLayout {
      static void resetTopLevel(QLayout * l) {
         auto d = static_cast<QLayoutPrivate*>(static_cast<LayoutHelper*>(l)->d_ptr.data());
         d->topLevel = false;
      }
   };
public:
   static QLayout * takeLayout(QWidget * w) {
      auto d = static_cast<QWidgetPrivate*>(static_cast<WidgetHelper*>(w)->d_ptr.data());
      auto l = w->layout();
      if (!l) return nullptr;
      d->layout = 0;
      l->setParent(nullptr);
      LayoutHelper::resetTopLevel(l);
      return l;
   }
};
QLayout * takeLayout(QWidget * w) { return WidgetHelper::takeLayout(w); }
