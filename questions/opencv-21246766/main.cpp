// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-21246766
#include <QtWidgets>
#include <algorithm>
#include <opencv2/opencv.hpp>

Q_DECLARE_METATYPE(cv::Mat)

struct AddressTracker {
   const void *address = {};
   int reallocs = 0;
   void track(const cv::Mat &m) { track(m.data); }
   void track(const QImage &img) { track(img.bits()); }
   void track(const void *data) {
      if (data && data != address) {
         address = data;
         reallocs ++;
      }
   }
};

class Capture : public QObject {
   Q_OBJECT
   Q_PROPERTY(cv::Mat frame READ frame NOTIFY frameReady USER true)
   cv::Mat m_frame;
   QBasicTimer m_timer;
   QScopedPointer<cv::VideoCapture> m_videoCapture;
   AddressTracker m_track;
public:
   Capture(QObject *parent = {}) : QObject(parent) {}
   ~Capture() { qDebug() << __FUNCTION__ << "reallocations" << m_track.reallocs; }
   Q_SIGNAL void started();
   Q_SLOT void start(int cam = {}) {
      if (!m_videoCapture)
         m_videoCapture.reset(new cv::VideoCapture(cam));
      if (m_videoCapture->isOpened()) {
         m_timer.start(0, this);
         emit started();
      }
   }
   Q_SLOT void stop() { m_timer.stop(); }
   Q_SIGNAL void frameReady(const cv::Mat &);
   cv::Mat frame() const { return m_frame; }
private:
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      if (!m_videoCapture->read(m_frame)) { // Blocks until a new frame is ready
         m_timer.stop();
         return;
      }
      m_track.track(m_frame);
      emit frameReady(m_frame);
   }
};

class Converter : public QObject {
   Q_OBJECT
   Q_PROPERTY(QImage image READ image NOTIFY imageReady USER true)
   Q_PROPERTY(bool processAll READ processAll WRITE setProcessAll)
   QBasicTimer m_timer;
   cv::Mat m_frame;
   QImage m_image;
   bool m_processAll = true;
   AddressTracker m_track;
   void queue(const cv::Mat &frame) {
      if (!m_frame.empty()) qDebug() << "Converter dropped frame!";
      m_frame = frame;
      if (! m_timer.isActive()) m_timer.start(0, this);
   }
   void process(const cv::Mat &frame) {
      Q_ASSERT(frame.type() == CV_8UC3);
      int w = frame.cols / 3.0, h = frame.rows / 3.0;
      if (m_image.size() != QSize{w,h})
         m_image = QImage(w, h, QImage::Format_RGB888);
      cv::Mat mat(h, w, CV_8UC3, m_image.bits(), m_image.bytesPerLine());
      cv::resize(frame, mat, mat.size(), 0, 0, cv::INTER_AREA);
      cv::cvtColor(mat, mat, CV_BGR2RGB);
      emit imageReady(m_image);
   }
   void timerEvent(QTimerEvent *ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      process(m_frame);
      m_frame.release();
      m_track.track(m_frame);
      m_timer.stop();
   }
public:
   explicit Converter(QObject * parent = nullptr) : QObject(parent) {}
   ~Converter() { qDebug() << __FUNCTION__ << "reallocations" << m_track.reallocs; }
   bool processAll() const { return m_processAll; }
   void setProcessAll(bool all) { m_processAll = all; }
   Q_SIGNAL void imageReady(const QImage &);
   QImage image() const { return m_image; }
   Q_SLOT void processFrame(const cv::Mat &frame) {
      if (m_processAll) process(frame); else queue(frame);
   }
};

class ImageViewer : public QWidget {
   Q_OBJECT
   Q_PROPERTY(QImage image READ image WRITE setImage USER true)
   bool painted = true;
   QImage m_img;
   AddressTracker m_track;
   void paintEvent(QPaintEvent *) {
      QPainter p(this);
      if (!m_img.isNull()) {
         setAttribute(Qt::WA_OpaquePaintEvent);
         p.drawImage(0, 0, m_img);
         painted = true;
      }
   }
public:
   ImageViewer(QWidget * parent = nullptr) : QWidget(parent) {}
   ~ImageViewer() { qDebug() << __FUNCTION__ << "reallocations" << m_track.reallocs; }
   Q_SLOT void setImage(const QImage &img) {
      if (!painted) qDebug() << "Viewer dropped frame!";
      if (m_img.size() == img.size() && m_img.format() == img.format()
          && m_img.bytesPerLine() == img.bytesPerLine())
         std::copy_n(img.bits(), img.sizeInBytes(), m_img.bits());
      else
         m_img = img.copy();
      painted = false;
      if (m_img.size() != size()) setFixedSize(m_img.size());
      m_track.track(m_img);
      update();
   }
   QImage image() const { return m_img; }
};

class Thread final : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
   qRegisterMetaType<cv::Mat>();
   QApplication app(argc, argv);
   ImageViewer view;
   Capture capture;
   Converter converter;
   Thread captureThread, converterThread;
   // Everything runs at the same priority as the gui, so it won't supply useless frames.
   converter.setProcessAll(false);
   captureThread.start();
   converterThread.start();
   capture.moveToThread(&captureThread);
   converter.moveToThread(&converterThread);
   QObject::connect(&capture, &Capture::frameReady, &converter, &Converter::processFrame);
   QObject::connect(&converter, &Converter::imageReady, &view, &ImageViewer::setImage);
   view.show();
   QObject::connect(&capture, &Capture::started, [](){ qDebug() << "Capture started."; });
   QMetaObject::invokeMethod(&capture, "start");
   return app.exec();
}

#include "main.moc"
