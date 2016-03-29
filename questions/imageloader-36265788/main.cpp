// https://github.com/KubaO/stackoverflown/tree/master/questions/imageloader-36265788
#include <QtWidgets>
#include <QtConcurrent>

class ImageSource : public QObject {
   Q_OBJECT
public:
   /// This method is thread-safe.
   void generate() {
      static auto counter = 0;
      QImage img(128, 128, QImage::Format_ARGB32);
      img.fill(Qt::white);
      QPainter p(&img);
      p.drawText(img.rect(), Qt::AlignCenter, QString::number(counter++));
      p.end();
      emit hasImage(img);
   }
   Q_SIGNAL void hasImage(const QImage & image);
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   ImageSource source;
   QLabel label;
   label.show();

   QObject::connect(&source, &ImageSource::hasImage, &label, [&](const QImage & image){
      label.setFixedSize(image.size());
      label.setPixmap(QPixmap::fromImage(image));
      QtConcurrent::run(&source, &ImageSource::generate);
   });

   QtConcurrent::run(&source, &ImageSource::generate); // generate the first image
   return app.exec();
}

#include "main.moc"
