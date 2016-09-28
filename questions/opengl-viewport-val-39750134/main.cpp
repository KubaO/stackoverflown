// https://github.com/KubaO/stackoverflown/tree/master/questions/opengl-viewport-val-39750134

#if 1

#include <QtGui>
#include <QtOpenGL>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
using QOpenGLWidget = QGLWidget;
#endif

class MyGraphicsView : public QGraphicsView {
   QOpenGLWidget m_gl;
public:
   MyGraphicsView(QWidget * parent = nullptr) : QGraphicsView{parent} {
      setViewport(&m_gl); // sets m_gl's parent
      Q_ASSERT(m_gl.parent());
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MyGraphicsView view;
   QGraphicsScene scene;
   scene.addText("Hello World");
   view.setScene(&scene);
   view.show();
   return app.exec();
}

#endif

#if 0

#include <QtGui>
#include <QtOpenGL>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
using QOpenGLWidget = QGLWidget;
#endif

// Don't code like this. It's silly.
class MyGraphicsView : public QGraphicsView {
   QOpenGLWidget * m_gl = new QOpenGLWidget;
public:
   MyGraphicsView(QWidget * parent = nullptr) : QGraphicsView{parent}
   {
      setViewport(m_gl); // sets m_gl's parent
      Q_ASSERT(m_gl->parent());
   }
   ~MyGraphicsView() {
      delete m_gl; // completely unnecessary
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MyGraphicsView view;
   QGraphicsScene scene;
   scene.addText("Hello World");
   view.setScene(&scene);
   view.show();
   return app.exec();
}


#endif
