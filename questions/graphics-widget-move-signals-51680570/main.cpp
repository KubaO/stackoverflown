// https://github.com/KubaO/stackoverflown/tree/master/questions/graphics-widget-move-signals-51680570
#include <QtWidgets>

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QGraphicsScene scene;
   QGraphicsView view(&scene);
   view.setAlignment(Qt::AlignLeft | Qt::AlignTop);
   scene.setSceneRect(0, 0, 1, 1);

   QGraphicsWidget parent;
   parent.setPos(150, 100);
   QLabel label("Select This");
   label.setContentsMargins(10, 10, 10, 10);
   auto *proxy = scene.addWidget(&label);
   proxy->setParentItem(&parent);
   parent.setFlags(QGraphicsItem::ItemIsMovable);
   scene.addItem(&parent);

   QGraphicsTextItem text;
   scene.addItem(&text);
   auto const updateText = [&] {
      text.setPlainText(QString("%1, %2").arg(parent.x()).arg(parent.y()));
   };
   QObject::connect(&parent, &QGraphicsObject::xChanged, &text, updateText);
   QObject::connect(&parent, &QGraphicsObject::yChanged, &text, updateText);
   updateText();

   view.setMinimumSize(320, 320);
   view.show();
   return a.exec();
}
