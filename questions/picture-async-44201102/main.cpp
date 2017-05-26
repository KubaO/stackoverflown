// https://github.com/KubaO/stackoverflown/tree/master/questions/picture-async-44201102
#include <QtWidgets>
#include <QtConcurrent>

class PictureSource : public QObject {
   Q_OBJECT
public:
   QPicture draw() {
      QPicture pic;
      QPainter p(&pic);
      p.rotate(QTime::currentTime().msec()*360./1000.);
      p.drawLine(0, 0, 50, 50);
      pic.setBoundingRect({-50, -50, 100, 100});
      emit pictureChanged(pic);
      return pic;
   }
   Q_SIGNAL void pictureChanged(const QPicture &);
};
Q_DECLARE_METATYPE(QPicture)

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   qRegisterMetaType<QPicture>();
   QLabel label;
   PictureSource source;
   QObject::connect(&source, &PictureSource::pictureChanged,
                    &label, &QLabel::setPicture);
   source.draw();
   label.show();
   QTimer timer;
   timer.start(50); // every 50 ms
   if (false)
      QObject::connect(&timer, &QTimer::timeout, &source, &PictureSource::draw);
   else
      QObject::connect(&timer, &QTimer::timeout, [&source]{
        auto pool = QThreadPool::globalInstance();
        QtConcurrent::run(pool, [&source]{ source.draw(); });
      });
   return app.exec();
}
#include "main.moc"
