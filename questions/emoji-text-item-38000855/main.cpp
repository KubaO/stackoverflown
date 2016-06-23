// https://github.com/KubaO/stackoverflown/tree/master/questions/emoji-text-item-38000855
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QGraphicsScene scene;
   QGraphicsView view{&scene};
   QGraphicsTextItem item{QStringLiteral("I'm a happy 👧. 😀")};
   scene.addItem(&item);
   view.ensureVisible(scene.sceneRect());
   view.show();
   return app.exec();
}
