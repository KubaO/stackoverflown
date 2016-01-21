#if 0
// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-view-33508582
#include <QtWidgets>

class MainWindow : public QMainWindow {
  Q_OBJECT
  QGraphicsScene m_scene;
  QWidget m_central;
  QGraphicsView m_view; // must be declared after m_central per C++ semantics
  QGridLayout m_layout;
public:
  MainWindow(QWidget * parent = 0) :
    QMainWindow(parent),
    m_layout(&m_central) {
    setCentralWidget(&m_central);
    m_layout.addWidget(&m_view, 0, 0);
    m_view.setScene(&m_scene);
  }
  QGraphicsScene * scene() { return &m_scene; }
  QGraphicsView * view() { return &m_view; }
};

int main(int argc, char ** argv) {
  QApplication app(argc, argv);
  MainWindow win;
  win.scene()->addEllipse(0, 0, 10, 10);
  win.show();
  return app.exec();
}

#include "main.moc"

#else

// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-view-33508582
#include <QtWidgets>

class MainWindow : public QDialog {
  Q_OBJECT
  QGraphicsScene m_scene;
  QGraphicsView m_view;
  QGridLayout m_layout;
public:
  MainWindow(QWidget * parent = 0) :
    QDialog(parent),
    m_layout(this) {
    m_layout.addWidget(&m_view, 0, 0);
    m_view.setScene(&m_scene);
  }
  QGraphicsScene * scene() { return &m_scene; }
  QGraphicsView * view() { return &m_view; }
};

int main(int argc, char ** argv) {
  QApplication app(argc, argv);
  MainWindow win;
  win.scene()->addEllipse(0, 0, 10, 10);
  win.show();
  return app.exec();
}

#include "main.moc"

#endif
