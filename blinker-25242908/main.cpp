#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QImage>
#include <QPainter>

void blink(QLabel * label, const QList<QImage> & images)
{
  const char * const prop = "imageIndex";
  Q_ASSERT(!images.isEmpty());
  if (label->property(prop).isNull()) {
    // We're setting the image for the first time
    label->setProperty(prop, images.size());
  }
  int i = (label->property(prop).toInt() + 1) % images.size();
  label->setPixmap(QPixmap::fromImage(images[i]));
  label->setProperty(prop, i);
}

QImage textImage(const QString & text, int size = 64)
{
  QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  QPainter p(&image);
  p.setFont(QFont("helvetica", 20));
  QTextOption opt;
  opt.setAlignment(Qt::AlignCenter);
  p.drawText(image.rect(), text, opt);
  return image;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QList<QImage> images;
  QLabel label;
  QTimer timer;

  images << textImage("0") << textImage("1") << textImage("2") << textImage("3");
  blink(&label, images);
  timer.start(250);
  QObject::connect(&timer, &QTimer::timeout, [&]{ blink(&label, images); });

  label.show();
  return a.exec();
}
