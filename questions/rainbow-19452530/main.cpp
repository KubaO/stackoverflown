// https://github.com/KubaO/stackoverflown/tree/master/questions/rainbow-19452530
#include <QtGui>
#if QT_VERSION_MAJOR > 4
#include <QtWidgets>
#endif
#include <array>
#include <cmath>

QColor wavelengthToColor(qreal lambda) {
   // Based on: http://www.efg2.com/Lab/ScienceAndEngineering/Spectra.htm
   // The foregoing is based on: http://www.midnightkite.com/color.html
   struct Color {
      qreal red, green, blue;
      QColor toColor(qreal factor) const {
         auto const map = [factor](qreal c) -> qreal {
            constexpr qreal gamma = 0.8;
            return pow(c * factor, gamma);
         };
         return QColor::fromRgbF(map(red), map(green), map(blue));
      }
   } color{};

   struct Threshold {
      qreal l1, l2;
      Color (*color)(qreal);
      qreal (*factor)(qreal) = nullptr;
   };
   static const std::array<Threshold, 8> thresholds{
      Threshold{380, 420, nullptr, [](qreal x){ return 0.3 + 0.7*x; }},
      Threshold{380, 440, [](qreal x){ return Color{1-x, 0, 1}; }},
      Threshold{440, 490, [](qreal x){ return Color{0, x, 1}; }},
      Threshold{490, 510, [](qreal x){ return Color{0, 1, 1-x}; }},
      Threshold{510, 580, [](qreal x){ return Color{x, 1, 0}; }},
      Threshold{580, 645, [](qreal x){ return Color{1, 1-x, 0}; }},
      Threshold{645, 780, [](qreal  ){ return Color{1, 0, 0}; }},
      Threshold{700, 780, nullptr, [](qreal x){ return 1 - 0.7*x; }}
   };

   qreal factor = 1.0;
   for (auto &thr : thresholds) {
      if (lambda < thr.l1 || lambda >= thr.l2) continue;
      auto x = (thr.l1 - lambda) / (thr.l1 - thr.l2);
      if (thr.factor) factor = thr.factor(x);
      // To let the intensity fall off near the vision limits
      if (thr.color) color = thr.color(x);
   }

   return color.toColor(factor);
}

QPixmap rainbow(int w, int h) {
   QPixmap pixmap(w, h);
   QPainter p(&pixmap);
   qreal f1 = 1.0 / 400;
   qreal f2 = 1.0 / 780;
   for (int x = 0; x < w; ++x) {
      // Iterate across frequencies, not wavelengths
      qreal lambda = 1.0 / (f1 - (x / qreal(w) * (f1 - f2)));
      p.setPen(wavelengthToColor(lambda));
      p.drawLine(x, 0, x, h);
   }
   return pixmap;
}

class RainbowLabel : public QLabel {
  protected:
   void resizeEvent(QResizeEvent *) override { setPixmap(rainbow(width(), height())); }
};

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   RainbowLabel l;
   l.resize(600, 100);
   l.show();
   return a.exec();
}
