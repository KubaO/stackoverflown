// https://github.com/KubaO/stackoverflown/tree/master/questions/wavelet-plot-50690312
#include <QtCharts>
#include <cmath>

const double pi = 3.14159265358979323846;

QVector<QPointF> ricker(double f, double length = 2.0, double dt = 0.001) {
   QVector<QPointF> w;
   size_t N = (length - dt/2.0)/dt;
   w.resize(N);
   for (size_t i = 0; i < N; ++i) {
      double t = -length/2 + i*dt;
      w[i].setX(t);
      w[i].setY((1.0 - 2*pi*pi*f*f*t*t) * exp(-pi*pi*f*f*t*t));
   }
   return w;
}

QLineSeries *rickerSeries(double f) {
   auto *series = new QLineSeries;
   series->setName(QStringLiteral("Ricker Wavelet for f=%1").arg(f, 2));
   series->replace(ricker(f));
   return series;
}

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QChartView view;
   view.chart()->addSeries(rickerSeries(1.0));
   view.chart()->addSeries(rickerSeries(2.0));
   view.chart()->createDefaultAxes();
   view.setMinimumSize(800, 600);
   view.show();
   return app.exec();
}
