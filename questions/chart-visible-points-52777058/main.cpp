// https://github.com/KubaO/stackoverflown/tree/master/questionschart-visible-points-52777058
#include <QtCharts>
#include <algorithm>
#include <cmath>

auto seriesRect(QChart *chart, QAbstractSeries *series = nullptr) {
   auto inScene = chart->plotArea();
   auto inChart = chart->mapFromScene(inScene);
   auto inChartRect = inChart.boundingRect();
   auto inItem1 = chart->mapToValue(inChartRect.topLeft(), series);
   auto inItem2 = chart->mapToValue(inChartRect.bottomRight(), series);
   return QRectF(inItem1, inItem2).normalized();
}

auto pointsInRect(QXYSeries *series, const QRectF &rect) {
   QVector<QPointF> result;
   auto const points = series->pointsVector();
   std::copy_if(points.begin(), points.end(), std::back_inserter(result),
                [rect](auto &p) { return rect.contains(p); });
   return result;
}

auto data() {
   QVector<QPointF> result;
   for (auto x = 0.; x < 10.; x += 0.1) result.append({x, exp(x)});
   return result;
}

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QWidget ui;
   QVBoxLayout layout(&ui);
   QChartView view1, view2;
   QLabel status;
   layout.addWidget(&view1);
   layout.addWidget(&view2);
   layout.addWidget(&status);
   layout.setMargin(4);

   QLineSeries series;
   series.replace(data());
   auto *chart = view1.chart();
   chart->addSeries(&series);
   view1.setRubberBand(QChartView::RectangleRubberBand);

   QLineSeries subSeries;
   subSeries.setPointsVisible(true);
   auto *subChart = view2.chart();
   subChart->addSeries(&subSeries);

   for (auto *chart : {view1.chart(), view2.chart()}) {
      chart->legend()->hide();
      chart->createDefaultAxes();
      chart->layout()->setContentsMargins(0, 0, 0, 0);
   }

   auto update = [&] {
      auto rect = seriesRect(chart, &series);
      auto const points = pointsInRect(&series, rect);

      status.setText(QStringLiteral("Visible Range: (%1,%2)-(%3,%4)")
                         .arg(rect.left())
                         .arg(rect.top())
                         .arg(rect.right())
                         .arg(rect.bottom()));

      subSeries.replace(points);
      subChart->axisX(&subSeries)->setRange(rect.left(), rect.right());
      subChart->axisY(&subSeries)->setRange(rect.top(), rect.bottom());
   };
   QObject::connect(chart, &QChart::plotAreaChanged, update);
   ui.setMinimumSize(400, 400);
   ui.show();
   return a.exec();
}
