// https://github.com/KubaO/stackoverflown/tree/master/questions/asi-astro-cam-39968889
#include <QtOpenGL>
#include <QOpenGLFunctions_2_0>
#include "ASICamera2.h"

class ASICamera : public QObject {
   Q_OBJECT
   ASI_ERROR_CODE m_error;
   ASI_CAMERA_INFO m_info;
   QImage m_frame{640, 480, QImage::Format_RGB888};
   QTimer m_timer{this};
   int m_exposure_ms = 0;
   inline int id() const { return m_info.CameraID; }
   void capture() {
      m_error = ASIGetVideoData(id(), m_frame.bits(), m_frame.byteCount(),
                                 m_exposure_ms*2 + 500);
      if (m_error == ASI_SUCCESS)
         emit newFrame(m_frame);
      else
         qDebug() << "capture error" << m_error;
   }
public:
   explicit ASICamera(QObject * parent = nullptr) : QObject{parent} {
      connect(&m_timer, &QTimer::timeout, this, &ASICamera::capture);
   }
   ASI_ERROR_CODE error() const { return m_error; }
   bool open(int index) {
      m_error = ASIGetCameraProperty(&m_info, index);
      if (m_error != ASI_SUCCESS)
         return false;
      m_error = ASIOpenCamera(id());
      if (m_error != ASI_SUCCESS)
         return false;
      m_error = ASIInitCamera(id());
      if (m_error != ASI_SUCCESS)
         return false;
      m_error = ASISetROIFormat(id(), m_frame.width(), m_frame.height(), 1, ASI_IMG_RGB24);
      if (m_error != ASI_SUCCESS)
         return false;
      return true;
   }
   bool close() {
      m_error = ASICloseCamera(id());
      return m_error == ASI_SUCCESS;
   }
   Q_SIGNAL void newFrame(const QImage &);
   QImage frame() const { return m_frame; }
   Q_SLOT bool start() {
      m_error = ASIStartVideoCapture(id());
      if (m_error == ASI_SUCCESS)
         m_timer.start(0);
      return m_error == ASI_SUCCESS;
   }
   Q_SLOT bool stop() {
      m_error = ASIStopVideoCapture(id());
      return m_error == ASI_SUCCESS;
      m_timer.stop();
   }
   ~ASICamera() {
      stop();
      close();
   }
};

class GLViewer : public QOpenGLWidget, protected QOpenGLFunctions_2_0 {
   Q_OBJECT
   QImage m_image;
   void ck() {
      for(GLenum err; (err = glGetError()) != GL_NO_ERROR;) qDebug() << "gl error" << err;
   }
   void initializeGL() override {
      initializeOpenGLFunctions();
      glClearColor(0.2f, 0.2f, 0.25f, 1.f);
   }
   void resizeGL(int width, int height) override {
      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, width, height, 0, 0, 1);
      glMatrixMode(GL_MODELVIEW);
      update();
   }
   // From http://stackoverflow.com/a/8774580/1329652
   void paintGL() override {
      auto scaled = m_image.size().scaled(this->size(), Qt::KeepAspectRatio);
      GLuint texID;
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glGenTextures(1, &texID);
      glEnable(GL_TEXTURE_RECTANGLE);
      glBindTexture(GL_TEXTURE_RECTANGLE, texID);
      glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, m_image.width(), m_image.height(), 0,
                   GL_RGB, GL_UNSIGNED_BYTE, m_image.constBits());

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2f(0, 0);
      glTexCoord2f(m_image.width(), 0);
      glVertex2f(scaled.width(), 0);
      glTexCoord2f(m_image.width(), m_image.height());
      glVertex2f(scaled.width(), scaled.height());
      glTexCoord2f(0, m_image.height());
      glVertex2f(0, scaled.height());
      glEnd();
      glDisable(GL_TEXTURE_RECTANGLE);
      glDeleteTextures(1, &texID);
      ck();
   }
public:
   GLViewer(QWidget * parent = nullptr) : QOpenGLWidget{parent} {}
   void setImage(const QImage & image) {
      Q_ASSERT(image.format() == QImage::Format_RGB888);
      m_image = image;
      update();
   }
};

class Thread : public QThread { public: ~Thread() { quit(); wait(); } };

// See http://stackoverflow.com/q/21646467/1329652
template <typename F>
static void postToThread(F && fun, QObject * obj = qApp) {
   QObject src;
   QObject::connect(&src, &QObject::destroyed, obj, std::move(fun), Qt::QueuedConnection);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   GLViewer viewer;
   viewer.setMinimumSize(200, 200);
   ASICamera camera;
   Thread thread;
   QObject::connect(&camera, &ASICamera::newFrame, &viewer, &GLViewer::setImage);
   QObject::connect(&thread, &QThread::destroyed, [&]{ camera.moveToThread(app.thread()); });
   camera.moveToThread(&thread);
   thread.start();
   postToThread([&]{
      camera.open(0);
      camera.start();
   }, &camera);
   viewer.show();
   return app.exec();
}
#include "main.moc"
