// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-polygon-7727656
#include <QtWidgets>

class MainWindow : public QWidget
{
   Q_OBJECT
   QGridLayout m_layout{this};
   QPushButton m_erase{"Erase"};
   QGraphicsView m_view;
public:
   MainWindow() {
      m_layout.addWidget(&m_erase, 0, 1);
      m_layout.addWidget(&m_view, 1, 0, 1, 3);
      m_view.setBackgroundBrush(Qt::black);
      connect(&m_erase, &QPushButton::clicked, this, &MainWindow::clearScene);
   }
   void setScene(QGraphicsScene * scene) {
      m_view.setScene(scene);
   }
   Q_SIGNAL void clearScene();
};

class MyScene : public QGraphicsScene {
   Q_OBJECT
   QPainterPath m_path;
   QGraphicsPathItem m_pathItem;
   QGraphicsLineItem m_lineItem;
   struct PathUpdater {
      QGraphicsPathItem & item;
      QPainterPath & path;
      PathUpdater(QGraphicsPathItem & item, QPainterPath & path) :
         item(item), path(path) {
         item.setPath({}); // avoid a copy-on-write
      }
      ~PathUpdater() { item.setPath(path); }
   };
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
      PathUpdater updater(m_pathItem, m_path);
      auto pos = event->scenePos();
      m_lineItem.setLine(0, 0, pos.x(), pos.y());
      m_lineItem.setVisible(true);
      if (m_path.elementCount() == 0)
         m_path.moveTo(pos);
      m_path.lineTo(pos.x()+1,pos.y()+1); // otherwise lineTo is a NOP
   }
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
      PathUpdater updater(m_pathItem, m_path);
      auto pos = event->scenePos();
      m_lineItem.setLine(0, 0, pos.x(), pos.y());
      m_path.setElementPositionAt(m_path.elementCount()-1, pos.x(), pos.y());
   }
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override {
      m_lineItem.setVisible(false);
   }
public:
   MyScene() {
      addItem(&m_pathItem);
      addItem(&m_lineItem);
      m_pathItem.setPen({Qt::red});
      m_pathItem.setBrush(Qt::NoBrush);
      m_lineItem.setPen({Qt::white});
      m_lineItem.setVisible(false);
   }
   Q_SLOT void clear() {
      m_path = {};
      m_pathItem.setPath(m_path);
   }
};

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   MainWindow w;
   MyScene scene;
   w.setMinimumSize(600, 600);
   w.setScene(&scene);
   QObject::connect(&w, &MainWindow::clearScene, &scene, &MyScene::clear);
   w.show();
   return app.exec();
}
#include "main.moc"
