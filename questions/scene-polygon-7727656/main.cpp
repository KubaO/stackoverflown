// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-polygon-7727656
#include <QtWidgets>

class MainWindow : public QWidget
{
   Q_OBJECT
   QGridLayout m_layout{this};
   QPushButton m_draw{"Draw"};
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
   QPolygonF m_polygon;
   QGraphicsPolygonItem m_polygonItem;
   QGraphicsLineItem m_lineItem;
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
      auto pos = event->scenePos();
      m_lineItem.setLine(0, 0, pos.x(), pos.y());
      m_lineItem.setVisible(true);
      if (m_polygon.isEmpty())
         m_polygon.append(pos);
      m_polygon.append(pos);
      m_polygonItem.setPolygon(m_polygon);
   }
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
      auto pos = event->scenePos();
      m_lineItem.setLine(0, 0, pos.x(), pos.y());
      m_polygon.back() = pos;
      m_polygonItem.setPolygon(m_polygon);
   }
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override {
      m_lineItem.setVisible(false);
   }
public:
   MyScene() {
      addItem(&m_polygonItem);
      addItem(&m_lineItem);
      m_polygonItem.setPen({Qt::red});
      m_polygonItem.setBrush(Qt::NoBrush);
      m_lineItem.setPen({Qt::white});
      m_lineItem.setVisible(false);
   }
   Q_SLOT void clear() {
      m_polygon.clear();
      m_polygonItem.setPolygon(m_polygon);
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
