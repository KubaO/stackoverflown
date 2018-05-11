// https://github.com/KubaO/stackoverflown/tree/master/questions/qimage-ros-50262348
#include <QtWidgets>
#include <memory>
#include <string>
#include <vector>

// Minimal reimplementation of ROS

#define ROS_ERROR qFatal
namespace sensor_msgs {
namespace image_encodings {
const std::string MONO8{"mono8"}, BGR8{"bgr8"}, BGRA8{"bgra8"}, RGB8{"rgb8"}, RGBA8{"rgba8"};
} // image_encodings
struct Image {
   std::vector<quint8> data;
   std::string encoding;
   uint32_t height;
   uint32_t width;
};
using ImagePtr = std::shared_ptr<Image>;
using ImageConstPtr = std::shared_ptr<const Image>;
} // sensor_msgs

template <typename T, typename ...Args> T takeProperty(QObject *obj, const char *name, Args ...args) {
   auto prop = obj->property(name);
   if (prop.isValid() && prop.type() == qMetaTypeId<T>()) {
      obj->setProperty(name, {});
      auto const val = qvariant_cast<T>(prop);
      prop.setValue(T{});
      obj->setProperty(name, prop);
      return val;
   }
   return T{std::forward<Args>(args)...};
}

template <typename T> void setProperty(QObject *obj, const char *name, T &&val) {
   auto prop = obj->property(name);
   obj->setProperty(name, {});
   prop.setValue(std::forward<T>(val));
   obj->setProperty(name, prop);
}

template <typename T, typename ...Args> T reuseProperty(QObject *obj, const char *name, Args ...args) {
   auto prop = obj->property(name);
   if (prop.isValid() && prop.type() == qMetaTypeId<T>()) {
      obj->setProperty(name, {});
      if (prop.isDetached()) {
         auto const val = qvariant_cast<T>(prop);
         prop.setValue(T{});
         if (val.isDetached()) {
            obj->setProperty(name, prop);
            return val;
         }
      }
   }
   return T{std::forward<Args>(args)...};
}

namespace ros {
struct Subscriber {};
struct NodeHandle {
   template<class M, class T>
   Subscriber subscribe(const std::string &, uint32_t, void(T::*fun)(M), T *obj) {
      struct Thread : QThread {
         Thread(QObject*p):QThread(p){} ~Thread() override { quit(); wait(); } };
      static QPointer<Thread> thread = new Thread(qApp);
      thread->start(); // no-op if already started
      auto *timer = new QTimer;
      timer->start(1000/60);
      timer->moveToThread(thread);
      QObject::connect(timer, &QTimer::timeout, [timer, obj, fun]{
         auto const msec = QTime::currentTime().msecsSinceStartOfDay();
         auto val = timer->property("name");
         QVariant f;
         auto img = takeProperty<QImage>(timer, "image", 256, 256, QImage::Format_ARGB32_Premultiplied);
//         else img = {256, 256, QImage::Format_ARGB32_Premultiplied};
         Q_ASSERT(img.isDetached());
         qDebug() << val.isDetached() << timer->property("image").constData() << (void*)img.constBits();
//         QImage img{256, 256, QImage::Format_ARGB32_Premultiplied};
         img.fill(Qt::white);
         QPainter p{&img};
         constexpr int period = 3000;
         p.scale(img.width()/2.0, img.height()/2.0);
         p.translate(1.0, 1.0);
         p.rotate((msec % period) * 360.0/period);
         p.setPen({Qt::darkBlue, 0.1});
         p.drawLine(QLineF{{-1., 0.}, {1., 0.}});
         p.end();
         setProperty(timer, "image", img);
         img = std::move(img).convertToFormat(QImage::Format_RGB888).rgbSwapped();

         sensor_msgs::ImageConstPtr ptr{new sensor_msgs::Image{
               {img.constBits(), img.constBits() + img.sizeInBytes()},
               sensor_msgs::image_encodings::BGR8,
                     (uint32_t)img.height(), (uint32_t)img.width()}};
         (*obj.*fun)(ptr);
      });
      return {};
   }   
};
void init(int &, char **, const std::string &) {}
} // ros

// Interface

class MainWindow : public QLabel {
   Q_OBJECT
public:
   MainWindow(int argc, char** argv, QWidget *parent = {});
protected:
   Q_SLOT void setImageMsg(const sensor_msgs::ImageConstPtr&);
   Q_SIGNAL void newImageMsg(const sensor_msgs::ImageConstPtr&);
private:
   ros::Subscriber sub;
};

Q_DECLARE_METATYPE(sensor_msgs::ImageConstPtr)

// Implementation

static QImage toImageShare(const sensor_msgs::ImageConstPtr &msg) {
   using namespace sensor_msgs::image_encodings;
   QImage::Format format = {};
   if (msg->encoding == RGB8 || msg->encoding == BGR8)
      format = QImage::Format_RGB888;
   else if (msg->encoding == RGBA8 || msg->encoding == BGRA8)
      format = QImage::Format_RGBA8888_Premultiplied;
   else if (msg->encoding == MONO8)
      format = QImage::Format_Grayscale8;
   else
      return {};
   QImage img(msg->data.data(), msg->width, msg->height, format);
   if (msg->encoding == BGR8 || msg->encoding == BGRA8)
      img = std::move(img).rgbSwapped();
   return img;
}

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) : QLabel(parent) {
   qRegisterMetaType<sensor_msgs::ImageConstPtr>();
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
   connect(this, &MainWindow::newImageMsg, this, &MainWindow::setImageMsg);
#else
   connect(this, SIGNAL(newImageMsg(sensor_msgs::ImageConstPtr)), SLOT(setImageMsg(sensor_msgs::ImageConstPtr)));
#endif
   ros::init(argc,argv,"MainWindow");
   ros::NodeHandle n;
   sub = n.subscribe("/usb_cam/image_raw", 1, &MainWindow::newImageMsg, this);
}

void MainWindow::setImageMsg(const sensor_msgs::ImageConstPtr &msg) {
   auto img = toImageShare(msg);
   auto pix = QPixmap::fromImage(std::move(img));
   setPixmap(pix);
   resize(pix.size());
}

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   MainWindow w{argc, argv};
   w.show();
   return app.exec();
}
#include "main.moc"
