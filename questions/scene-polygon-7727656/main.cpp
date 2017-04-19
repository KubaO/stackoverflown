// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-polygon-7727656
#include <QtWidgets>

class MainWindow : public QWidget
{
   Q_OBJECT
   QGridLayout m_layout{this};
   QPushButton m_new{"New"};
   QPushButton m_erase{"Erase All"};
   QLabel m_label;
   QGraphicsView m_view;
public:
   MainWindow() {
      m_layout.addWidget(&m_new, 0, 0);
      m_layout.addWidget(&m_erase, 0, 1);
      m_layout.addWidget(&m_label, 0, 2);
      m_layout.addWidget(&m_view, 1, 0, 1, 3);
      m_view.setBackgroundBrush(Qt::black);
      m_view.setAlignment(Qt::AlignBottom | Qt::AlignLeft);
      m_view.scale(1, -1);
      connect(&m_new, &QPushButton::clicked, this, &MainWindow::newItem);
      connect(&m_erase, &QPushButton::clicked, this, &MainWindow::clearScene);
   }
   void setScene(QGraphicsScene * scene) {
      m_view.setScene(scene);
   }
   Q_SIGNAL void newItem();
   Q_SIGNAL void clearScene();
   Q_SLOT void setText(const QString & text) { m_label.setText(text); }
};

class MyScene : public QGraphicsScene {
   Q_OBJECT
public:
   struct Status {
      int paths;
      int elements;
   };
private:
   bool m_newItem = {};
   Status m_status = {0, 0};
   QPainterPath m_path;
   QGraphicsPathItem m_pathItem;
   QGraphicsLineItem m_lineItem;
   struct PathUpdater {
      Q_DISABLE_COPY(PathUpdater)
      MyScene & s;
      PathUpdater(MyScene & scene) : s(scene) {
         s.m_pathItem.setPath({}); // avoid a copy-on-write
      }
      ~PathUpdater() {
         s.m_pathItem.setPath(s.m_path);
         s.m_status = {0, s.m_path.elementCount()};
         for (auto i = 0; i < s.m_status.elements; ++i) {
            auto element = s.m_path.elementAt(i);
            if (element.type == QPainterPath::MoveToElement)
               s.m_status.paths++;
         }
         emit s.statusChanged(s.m_status);
      }
   };
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
      PathUpdater updater(*this);
      auto pos = event->scenePos();
      m_lineItem.setLine(0, 0, pos.x(), pos.y());
      m_lineItem.setVisible(true);
      if (m_path.elementCount() == 0 || m_newItem)
         m_path.moveTo(pos);
      m_path.lineTo(pos.x()+1,pos.y()+1); // otherwise lineTo is a NOP
      m_newItem = {};
   }
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
      PathUpdater updater(*this);
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
      PathUpdater updater(*this);
      m_path = {};
   }
   Q_SLOT void newItem() {
      m_newItem = true;
   }
   Q_SIGNAL void statusChanged(const MyScene::Status &);
   Status status() const { return m_status; }
};

int main(int argc, char *argv[])
{
   using Q = QObject;
   QApplication app{argc, argv};
   MainWindow w;
   MyScene scene;
   w.setMinimumSize(600, 600);
   w.setScene(&scene);
   Q::connect(&w, &MainWindow::clearScene, &scene, &MyScene::clear);
   Q::connect(&w, &MainWindow::newItem, &scene, &MyScene::newItem);
   auto onStatus = [&](const MyScene::Status & s){
      w.setText(QStringLiteral("Paths: %1 Elements: %2").arg(s.paths).arg(s.elements));
   };
   Q::connect(&scene, &MyScene::statusChanged, onStatus);
   onStatus(scene.status());
   w.show();
   return app.exec();
}
#include "main.moc"
