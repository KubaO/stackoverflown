// https://github.com/KubaO/stackoverflown/tree/master/questions/image-pad-35968431
#include <QtGui>

template <typename T>
QImage paddedImage(const QImage & source, int padWidth, T padValue) {
  QImage padded{source.width() + 2*padWidth, source.height() + 2*padWidth, source.format()};
  padded.fill(padValue);
  QPainter p{&padded};
  p.drawImage(QPoint(padWidth, padWidth), source);
  return padded;
}

int main() {
   QImage source{64, 64, QImage::Format_ARGB32_Premultiplied};
   source.fill(Qt::red);
   auto padded = paddedImage(source, 16, Qt::blue);
   padded.save("test.png");
}
