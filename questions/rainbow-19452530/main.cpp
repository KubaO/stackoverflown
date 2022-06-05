// https://github.com/KubaO/stackoverflown/tree/master/questions/rainbow-19452530
#include <QtGui>
#if QT_VERSION_MAJOR > 4
#include <QtWidgets>
#endif
#include <array>
#include <cmath>

constexpr qreal linMap(qreal x1, qreal x, qreal x2, qreal y1 = 0., qreal y2 = 1.) {
   return y1 + (y1 - y2) * (x - x1) / (x1 - x2);
}

QColor wavelengthToColor(qreal lambda) {
   // Based on: http://www.efg2.com/Lab/ScienceAndEngineering/Spectra.htm
   // The foregoing is based on: http://www.midnightkite.com/color.html
   struct Color {
      qreal red = 0., green = 0., blue = 0.;
      QColor toColor(qreal factor) const {
         auto const map = [factor](qreal c) -> qreal {
            constexpr qreal gamma = 0.8;
            return pow(c * factor, gamma);
         };
         return QColor::fromRgbF(map(red), map(green), map(blue));
      }
   };
   struct Threshold {
      qreal begin, end;
      Color (*color)(qreal);
      qreal (*factor)(qreal) = nullptr;
   };
   static const std::array<Threshold, 8> thresholds{
      // Let the intensity fall off near the vision limits
      Threshold{380, 420, nullptr, [](qreal x){ return 0.3 + 0.7*x; }},
      Threshold{380, 440, [](qreal x){ return Color{1-x, 0, 1}; }},
      Threshold{440, 490, [](qreal x){ return Color{0, x, 1}; }},
      Threshold{490, 510, [](qreal x){ return Color{0, 1, 1-x}; }},
      Threshold{510, 580, [](qreal x){ return Color{x, 1, 0}; }},
      Threshold{580, 645, [](qreal x){ return Color{1, 1-x, 0}; }},
      Threshold{645, 780, [](qreal  ){ return Color{1, 0, 0}; }},
      Threshold{700, 780, nullptr, [](qreal x){ return 1 - 0.7*x; }}
   };

   Color color;
   qreal factor = 1.0;
   for (auto &thr : thresholds) {
      if (lambda < thr.begin || lambda >= thr.end) continue;
      auto x = linMap(thr.begin, lambda, thr.end);
      if (thr.factor) factor = thr.factor(x);
      if (thr.color) color = thr.color(x);
   }
   return color.toColor(factor);
}

QPixmap rainbow(int w, int h) {
   QPixmap pixmap(w, h);
   QPainter p(&pixmap);
   constexpr qreal f1 = 1.0 / 400, f2 = 1.0 / 780;
   for (int x = 0; x < w; ++x) {
      // Iterate across frequencies, not wavelengths
      auto freq = linMap(0, x, w, f1, f2);
      p.setPen(wavelengthToColor(1.0 / freq));
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
