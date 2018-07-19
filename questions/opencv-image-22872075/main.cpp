// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-image-22872075
// This project is compatible with Qt 4 and Qt 5
#ifdef MACPORTS_QT4_BINARY_COMPAT_FIX
#define QT3_SUPPORT
#endif
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <private/qimage_p.h>
#endif
#include <opencv2/opencv.hpp>

namespace compat { // c.f. https://stackoverflow.com/q/49862299/1329652
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
using QT_PREPEND_NAMESPACE(QImage);
#else
using Q_QImage = QT_PREPEND_NAMESPACE(QImage);
using Q_QImageData = QT_PREPEND_NAMESPACE(QImageData);
class QImageData : public Q_QImageData {
public:
   void (*cleanupFunction)(void*) = {};
   void *cleanupInfo = {};
   explicit QImageData(const Q_QImageData &o) {
      qDebug() << "dup from" << &o << "to" << this;
      *(Q_QImageData*)(this) = o;
      qDebug() << "dup done";
   }
   ~QImageData() {
      qDebug() << __FUNCTION__;
      if (cleanupFunction)
         cleanupFunction(cleanupInfo);
   }
};
class QImage : public Q_QImage {
   enum { ReplacedFlag = 0x0EDA23C0, ReplacedMask = 0x0FFFFFF0 };
   int &flags() {
      return *reinterpret_cast<int*>((&data_ptr()->offset) + 1);
   }
   bool isReplaced() { return (flags() & ReplacedMask) == ReplacedFlag; }
   void setReplaced(bool r) { flags() = (flags() & ~ReplacedMask) | (r ? ReplacedFlag : 0); }
public:
   using Q_QImage::Q_QImage;
   QImage(uchar *data, int width, int height, int bytesPerLine, Format format,
          void (*cleanupFunction)(void*) = {}, void *cleanupInfo = {}) :
      Q_QImage(data, width, height, bytesPerLine, format)
   {
      qDebug() << __FUNCTION__;
      Q_ASSERT(!data_ptr() || !isReplaced());
      if (data_ptr()) {
         bool own_data = data_ptr()->own_data;
         bool ro_data = data_ptr()->ro_data;
         bool has_alpha_clut = data_ptr()->has_alpha_clut;
         bool is_cached = data_ptr()->is_cached;
         auto *d1 = new QImageData(*data_ptr());
         d1->cleanupFunction = cleanupFunction;
         d1->cleanupInfo = cleanupInfo;
         auto *d2 = data_ptr();
         data_ptr() = d1;
         d2->colortable.~QVector();
   #ifndef QT_NO_IMAGE_TEXT
         //d2->text.~QMap();
   #endif
         operator delete(d2); // can't invoke the destructor as it would free the data
         setReplaced(true);
         Q_ASSERT(own_data == d1->own_data);
         Q_ASSERT(ro_data == d1->ro_data);
         Q_ASSERT(has_alpha_clut == d1->has_alpha_clut);
         Q_ASSERT(is_cached == d1->is_cached);
      }
   }
   ~QImage() override {
      qDebug() << __FUNCTION__;
      auto d = static_cast<QImageData*>(data_ptr());
      if (d && isReplaced()) {
         if (!d->ref.deref()) {
            delete d;
            data_ptr() = nullptr;
         } else
            d->ref.ref(); // prevent the QImage destructor from freeing the wrong type
      }
   }
};
#endif
}

QImage img;

compat::QImage imageFromMat(cv::Mat const& src) {
   Q_ASSERT(src.type() == CV_8UC3);
   auto *mat = new cv::Mat(src.cols,src.rows,src.type());
   cvtColor(src, *mat, CV_BGR2RGB);
   return compat::QImage((uchar*)mat->data, mat->cols, mat->rows, mat->step,
                         QImage::Format_RGB888,
                         [](void *mat){ delete static_cast<cv::Mat*>(mat); }, mat);
}

QPixmap pixmapFromMat(const cv::Mat &src) {
   auto image(imageFromMat(src));
   return QPixmap::fromImage(image);
}

static cv::Scalar randomScalar() {
   static cv::RNG rng(12345);
   return cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
}

class UIQT : public QWidget {
   Q_OBJECT
   QGridLayout m_layout{this};
   QLabel m_label;
   QBasicTimer m_timer;
   cv::Size m_size{100, 100};
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() == m_timer.timerId())
         refreshImage();
   }
public:
   UIQT(QWidget *parent = {}) : QWidget(parent) {
      m_layout.addWidget(&m_label, 0, 0);
      m_timer.start(500, this);
      refreshImage();
   }
   Q_SLOT void refreshImage() {
      cv::Mat mat(m_size, CV_8UC3, randomScalar());
      m_label.setPixmap(pixmapFromMat(mat));
   }
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   compat::Q_QImageData d1;
   QImageData d2(d1);
   UIQT w;
   w.show();
   return app.exec();
}

#include "main.moc"
