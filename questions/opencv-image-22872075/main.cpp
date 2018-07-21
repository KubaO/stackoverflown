// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-image-22872075
// This project is compatible with Qt 4 and Qt 5
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <opencv2/opencv.hpp>




struct ConvData {
   QImage::Format dstFormat;
   int dstCode;
   int dstChannels() const { return (dstCode >> CV_CN_SHIFT) + 1; }
   int srcChannels;
   int code1;
   int code2;
   enum { SATALPHA = -2, TAKEALPHA = -1 };
};

const ConvData convData[] = {
   { QImage::Format_Invalid, 0, 0, 0, 0, },
   { QImage::Format_RGB32, CV_8U4, 1, cv::COLOR_GRAY2BGRA, -1 },
   { QImage::Format_RGB32, CV_8U4, 3, cv::COLOR_BGR2BGRA, -1 },
   { QImage::Format_RGB32, CV_8U4, 4, ConvData::SATALPHA, -1 },
   { QImage::Format_ARGB32, CV_8U4, 1, cv::COLOR_GRAY2BGRA, -1 },
   { QImage::Format_ARGB32, CV_8U4, 3, cv::COLOR_BGR2BGRA, -1 },
   { QImage::Format_ARGB32, CV_8U4, 4, -1, -1 },
   { QImage::Format_ARGB32_Premultiplied, CV_8U4, 1, cv::COLOR_GRAY2BGRA, -1 },
   { QImage::Format_ARGB32_Premultiplied, CV_8U4, 3, cv::COLOR_BGR2BGRA, -1 },
   { QImage::Format_ARGB32_Premultiplied, CV_8U4, 4, cv::COLOR_RGBA2mRGBA, -1 },
   { QImage::Format_RGBX8888, CV_8U4, 1, cv::COLOR_GRAY2RGBA, -1 },
   { QImage::Format_RGBX8888, CV_8U4, 3, cv::COLOR_BGR2RGBA, -1 },
   { QImage::Format_RGBX8888, CV_8U4, 4, ConvData::SATALPHA, cv::COLOR_BGRA2RGBA },
   { QImage::Format_RGBA8888, CV_8U4, 1, cv::COLOR_GRAY2RGBA, -1 },
   { QImage::Format_RGBA8888, CV_8U4, 3, cv::COLOR_BGR2RGBA, -1 },
   { QImage::Format_RGBA8888, CV_8U4, 4, cv::COLOR_BGRA2RGBA, -1 },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 1, cv::COLOR_GRAY2RGBA, -1 },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 3, cv::COLOR_BGR2RGBA, -1 },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 4, cv::COLOR_BGRA2RGBA, cv::COLOR_RGBA2mRGBA },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 1, cv::COLOR_GRAY2RGBA, -1 },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 3, cv::COLOR_BGR2RGBA, -1 },
   { QImage::Format_RGBA8888_Premultiplied, CV_8U4, 4, cv::COLOR_BGRA2RGBA, cv::COLOR_RGBA2mRGBA },
   { QImage::Format_RGB888, CV_8U3, 1, cv::COLOR_GRAY2BGR, -1 },
   { QImage::Format_RGB888, CV_8U3, 3, -1, -1 },
   { QImage::Format_RGB888, CV_8U3, 4, cv::COLOR_RGBA2BGR, -1 },
   { QImage::Format_RGB16, CV_16U1, 1, cv::COLOR_GRAY2BGR565, -1 },
   { QImage::Format_RGB16, CV_16U1, 3, cv::COLOR_BGR2BGR565, -1 },
   { QImage::Format_RGB16, CV_16U1, 4, cv::COLOR_RGBA2BGR565, -1 },
   { QImage::Format_RGB555, CV_16U1, 1, cv::COLOR_GRAY2BGR555, -1 },
   { QImage::Format_RGB555, CV_16U1, 3, cv::COLOR_BGR2BGR555, -1 },
   { QImage::Format_RGB555, CV_16U1, 4, cv::COLOR_RGBA2BGR555, -1 },
   { QImage::Format_Grayscale8, CV_8U1, 1, -1, -1 },
   { QImage::Format_Grayscale8, CV_8U1, 3, cv::COLOR_BGR2GRAY, -1 },
   { QImage::Format_Grayscale8, CV_8U1, 4, cv::COLOR_BGRA2GRAY, -1 },
   { QImage::Format_Alpha8, CV_8U1, 1, -1, -1 },
   { QImage::Format_Alpha8, CV_8U1, 3, ConvData::SATALPHA, -1 },
   { QImage::Format_Alpha8, CV_8U1, 4, ConvData::TAKEALPHA, -1 }
};

const ConvData &getConvData(const cv::Mat &m, QImage::Format format) {
   for (auto &d : convData)
      if (d.dst == format && d.srcChannels == m.channels())
         return d;
   return convData[0];
}

void convert(const cv::Mat &src, cv::Mat &dst, int code) {
   using El = cv::Vec4b;
   if (code == ConvData::SATALPHA) {
      for (auto it = dst.begin<El>(); it != dst.end<El>(); it++)
         (*it)[3] = 0xFF;
   }
   else if (code == ConvData::TAKEALPHA) {
      auto s = src.begin<El>();
      auto d = dst.begin<uchar>();
      for (; s != src.end<El>(); s++, d++)
         *d = (*s)[3] ;
   }
   else if (code != -1)
      cv::cvtColor(src, dst, code);
}

// Mat::convertTo - change depth
// cv::cvtColor - change channel count

QImage imageFromMat(cv::Mat src, QImage::Format format = QImage::Format_Invalid) {
   // By default, preserve the format
   if (format == QImage::Format_Invalid) {
      if (src.channels() == 1)
         format = QImage::Format_Grayscale8;
      else if (src.channels() == 3)
         format = QImage::Format_RGB888;
      else if (src.channels() == 4)
         format = QImage::Format_ARGB32;
   }
   auto data = getConvData(src, format);
   if (!src.data || !src.u || format == QImage::Format_Invalid ||
       data.dst == QImage::Format_Invalid)
      return {};

   QImage dst;
   cv::Mat dstMat_;
   cv::Mat *dstMat = &dstMat;

   bool keepBuffer = false;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
   keepBuffer = CV_XADD(&src.u->refcount, 0) == 1 // sole reference
         && (src.depth() == CV_8U || src.depth() == CV_8S)
         && src.channels() == data.dstChannels();
   if (keepBuffer) {
      dst = QImage((uchar*)src.data, src.cols, src.rows, src.step, data.dstFormat,
                   [](void *m){ delete static_cast<cv::Mat*>(m); }, new cv::Mat(src));
      dstMat = &src;
   }
#endif
   if (!keepBuffer) {
      dst = QImage(src.cols, src.rows, data.dstFormat);
      dstMat_ = cv::Mat(src.rows, src.cols, data.dstCode, dst.bits(), dst.bytesPerLine());
   }

   cv::Mat depthMat_;
   cv::Mat *depthMat = &depthMat;

   if (src.depth() == CV_8U || src.depth() == CV_8S || src.channels() == data.dstChannels())
      depthMat = &dst;

   double alpha = (src.depth == CV_)
   if (src.depth() != CV_8U)
      src.convertTo(src, CV_8U);


   return dst;
}

QPixmap pixmapFromMat(cv::Mat &&src) {
   auto image(imageFromMat(std::move(src)));
   return QPixmap::fromImage(image);
}

QPixmap pixmapFromMat(const cv::Mat &src) {
   auto image(imageFromMat(src));
   return QPixmap::fromImage(image);
}

static cv::Scalar randomScalar() {
   static cv::RNG rng(12345);
   return cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
}

class UIQT : public QWidget {
   Q_OBJECT
   QGridLayout m_layout{this};
   QLabel m_label;
   QBasicTimer m_timer;
   cv::Size m_size{200, 200};
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
      m_label.setPixmap(pixmapFromMat(std::move(mat)));
   }
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   UIQT w;
   w.show();
   return app.exec();
}

#include "main.moc"
