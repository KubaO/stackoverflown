#include <QApplication>
#include <QLabel>
#include <QImage>
#include <QPainter>

QImage icon(int size) {
   QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
   image.fill(Qt::transparent);
   QPainter p(&image);
   p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen);
   p.translate(image.rect().center());
   p.scale(image.width()/2.2, image.height()/2.2);
   p.setBrush(Qt::white);
   p.drawEllipse(QRectF(-.5, -.5, 1., 1.));
   p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
   p.setBrush(Qt::transparent);
   p.drawEllipse(QRectF(-.3, -.3, .6, .6));
   foreach (qreal angle, QList<qreal>() << 0. << 100. << 150.) {
      p.save();
      p.rotate(angle);
      p.drawRect(QRectF(-.1, 0, .2, -1.));
      p.restore();
   }
   return image;
}

QImage checkers(int size) {
   QImage img(size*2, size*2, QImage::Format_ARGB32_Premultiplied);
   QPainter p(&img);
   p.setPen(Qt::NoPen);
   p.fillRect(0, 0, size, size, Qt::darkGray);
   p.fillRect(size, size, size, 2*size, Qt::darkGray);
   p.fillRect(size, 0, size, size, Qt::lightGray);
   p.fillRect(0, size, size, size, Qt::lightGray);
   return img;
}

void drawColorIcon(QPainter & p, QColor color, const QImage & alpha)
{
  p.save();
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
  p.fillRect(QRect(0, 0, alpha.width(), alpha.height()), color);
  p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  p.drawImage(0, 0, alpha);
  p.restore();
}

QImage drawColorIconProof(QColor color, const QImage & alpha) {
   QImage result(alpha.size(), alpha.format());
   QPainter p(&result);
   drawColorIcon(p, color, alpha);
   p.setPen(Qt::NoPen);
   QBrush brush;
   brush.setTextureImage(checkers(10));
   p.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
   p.fillRect(alpha.rect(), brush);
   return result;
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QLabel label;
   label.setPixmap(QPixmap::fromImage(drawColorIconProof("orangered", icon(200))));
   label.show();
   return a.exec();
}
