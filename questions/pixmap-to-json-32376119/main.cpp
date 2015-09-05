#include <QtWidgets>

QJsonValue jsonValFromPixmap(const QPixmap & p) {
  QByteArray data;
  QBuffer buffer { &data };
  buffer.open(QIODevice::WriteOnly);
  p.save(&buffer, "PNG");
  auto encoded = buffer.data().toBase64();
  return QJsonValue(QString::fromLatin1(encoded));
}

QPixmap pixmapFrom(const QJsonValue & val) {
  QByteArray encoded = val.toString().toLatin1();
  QPixmap p;
  p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
  return p;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QImage img{32, 32, QImage::Format_RGB32};
   img.fill(Qt::red);
   auto pix = QPixmap::fromImage(img);
   auto val = jsonValFromPixmap(pix);
   auto pix2 = pixmapFrom(val);
   auto img2 = pix2.toImage();
   Q_ASSERT(img == img2);
}

