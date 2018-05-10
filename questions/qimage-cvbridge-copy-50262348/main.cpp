// https://github.com/KubaO/stackoverflown/tree/master/questions/qimage-cvbridge-copy-50262348
#include <QtWidgets>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <atomic>
#include <cstdint>

// Minimal reimplementation of ROS

#define ROS_ERROR qFatal
namespace sensor_msgs {
namespace image_encodings {
const std::string BGR8{"bgr8"};
} // image_encodings

struct Image {
   std::vector<uint8_t> data;
   std::string encoding;
   uint32_t height;
   uint32_t width;
};
using ImagePtr = boost::shared_ptr<Image>;
using ImageConstPtr = boost::shared_ptr<const Image>;
} // sensor_msgs

namespace ros {
struct Subscriber {
};
struct NodeHandle {
   template<class M, class T>
   Subscriber subscribe(const std::string &, uint32_t, void(T::*fun)(M), T *obj) {
      struct Thread : QThread { ~Thread() override { quit(); wait(); }};
      auto *timer = new QTimer(qApp);
      timer->start(1000/60);
      QObject::connect(timer, &QTimer::timeout, [obj, fun]{
         auto const msec = QTime::currentTime().msecsSinceStartOfDay();
         QImage img{256, 256, QImage::Format_RGB888};
         img.fill(Qt::white);
         QPainter p{&img};
         constexpr int period = 3000;
         p.scale(img.width()/2.0, img.height()/2.0);
         p.translate(1.0, 1.0);
         p.rotate((msec % period) * 360.0/period);
         p.setPen({Qt::darkBlue, 0.1});
         p.drawLine(QLineF{{-1., 0.}, {1., 0.}});
         p.end();
         sensor_msgs::ImageConstPtr ptr{new sensor_msgs::Image{
               {img.constBits(), img.constBits() + img.sizeInBytes()},
               sensor_msgs::image_encodings::BGR8,
                     (uint32_t)img.height(), (uint32_t)img.width()}};
         (*obj.*fun)(ptr);
      });
      return {};
   }
};
void 	init(int &, char **, const std::string &) {}
} // ros

class MainWindow : public QLabel {
   Q_OBJECT
public:
   MainWindow(int argc, char** argv, QWidget *parent = {});
protected:
   void callBackColor(const sensor_msgs::ImageConstPtr& msg);
   Q_SLOT void setImage(const QImage &);
   Q_SLOT void setImageMsg(const sensor_msgs::ImageConstPtr&);
   Q_SIGNAL void newImageMsg(const sensor_msgs::ImageConstPtr&);
private:
   ros::Subscriber sub;
};

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) : QLabel(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
   connect(this, &MainWindow::newImageMsg, this, &MainWindow::setImageMsg);
#else
   connect(this, Q_SIGNAL(newImageMsg(sensor_msgs::ImageConstPtr)), this, Q_SLOT(newImageMsg(sensor_msgs::ImageConstPtr)));
#endif

   ros::init(argc,argv,"MainWindow");
   ros::NodeHandle n;
   sub = n.subscribe("/usb_cam/image_raw", 1, &MainWindow::callBackColor, this);
}

void MainWindow::setImageMsg(const sensor_msgs::ImageConstPtr &msg) {
   QImage temp(msg->data.data(), msg->width, msg->height, QImage::Format_RGB888);
   setImage(temp);
}

void MainWindow::setImage(const QImage &image) {
   auto pix = QPixmap::fromImage(image);
   setPixmap(pix);
   resize(pix.size());
}

void MainWindow::callBackColor(const sensor_msgs::ImageConstPtr& msg)
{
   static const auto signal = QMetaMethod::fromSignal(&MainWindow::newImageMsg);
   if (isSignalConnected(signal))
      Q_EMIT newImageMsg(msg);
}

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   MainWindow w{argc, argv};
   w.show();
   return app.exec();
}
#include "main.moc"
