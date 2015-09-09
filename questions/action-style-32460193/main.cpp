// https://github.com/KubaO/stackoverflown/tree/master/questions/action-style-32460193
#include <QtWidgets>

void nameAction(QToolBar * bar, QAction * action, const char * name = nullptr) {
   if (name && action->objectName().isEmpty())
      action->setObjectName(name);
   bar->widgetForAction(action)->setObjectName(action->objectName());
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QToolBar tb;
   nameAction(&tb, tb.addAction("Foo"), "foo");
   nameAction(&tb, tb.addAction("Bar"), "bar");
   nameAction(&tb, tb.addAction("Baz"), "baz");
   tb.setStyleSheet(
            "QToolButton#foo { background:red }"
            "QToolButton#baz { background:blue }");
   tb.show();
   return app.exec();
}

