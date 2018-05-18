// https://github.com/KubaO/stackoverflown/tree/master/questions/painter-soft-pen-50411452
#include <QtWidgets>
#include <opencv2/imgproc.hpp>

cv::Mat matFor(const QImage &img) {
   Q_ASSERT(img.format() == QImage::Format_ARGB32_Premultiplied);
   return {img.height(), img.width(), CV_8UC4, (void*)img.bits(), (size_t)img.bytesPerLine()};
}

QImage makeSame(const QImage &as) {
   return {as.width(), as.height(), as.format()};
}

void blur(const QImage &src, QImage &dst, qreal sigmaX, qreal sigmaY = 0) {
   cv::GaussianBlur(matFor(src), matFor(dst), {}, sigmaX, sigmaY);
}

void blur(QImage &src, qreal sigmaX, qreal sigmaY = 0) {
   blur(src, src,  sigmaX, sigmaY);
}

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QImage img(512, 512, QImage::Format_ARGB32_Premultiplied);
   img.fill(Qt::white);
   QPainter p(&img);
   p.setPen({Qt::darkBlue, 4});
   p.translate(img.rect().center());
   p.drawLine(-200, -200, 200, 200);
   p.end();
   blur(img, 2);
   QLabel w;
   w.setPixmap(QPixmap::fromImage(img));
   w.show();
   return app.exec();
}
