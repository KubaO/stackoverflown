// https://github.com/KubaO/stackoverflown/tree/master/questions/pixmap-to-json-32376119
#include <QtGui>

QJsonValue jsonValFromPixmap(const QPixmap &p) {
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  p.save(&buffer, "PNG");
  auto const encoded = buffer.data().toBase64();
  return {QLatin1String(encoded)};
}

QPixmap pixmapFrom(const QJsonValue &val) {
  auto const encoded = val.toString().toLatin1();
  QPixmap p;
  p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
  return p;
}

int main(int argc, char **argv) {
   QGuiApplication app{argc, argv};
   QImage img{32, 32, QImage::Format_RGB32};
   img.fill(Qt::red);
   auto pix = QPixmap::fromImage(img);
   auto val = jsonValFromPixmap(pix);
   auto pix2 = pixmapFrom(val);
   auto img2 = pix2.toImage();
   Q_ASSERT(img == img2);
}
