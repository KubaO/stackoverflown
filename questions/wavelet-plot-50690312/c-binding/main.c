// https://github.com/KubaO/stackoverflown/tree/master/questions/wavelet-plot-50690312/c-binding
#include "qc_binding.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

const double pi = 3.14159265358979323846;

CQVector *ricker(double f, double length, double dt) {
   scope_enter();
   size_t N = (length - dt/2.0)/dt;
   CQVector *vector = CQVector_size_(CQ_, CQPointF_type(), N);
   CQPointF *const points = CQPointF_(CQVector_data_at(vector, 0));
   for (size_t i = 0; i < N; ++i) {
      double t = -length/2 + i*dt;
      points[i].x = t;
      points[i].y = (1.0 - 2*pi*pi*f*f*t*t) * exp(-pi*pi*f*f*t*t);
   }
   return scope_leave_ptr(vector);
}

CQLineSeries *rickerSeries(double f) {
   scope_enter();
   CQLineSeries *series = CQLineSeries_(CQ_);
   CQLineSeries_setName(series, CQString_asprintf(CQ_, "Ricker Wavelet for f=%.2f", f));
   CQLineSeries_replaceVector(series, ricker(f, 2.0, 0.001));
   return scope_leave_ptr(series);
}

int main(int argc, char *argv[]) {
   scope_enter();
   CQApplication *app = CQApplication_(CQ_, &argc, argv);
   CQChartView *view = CQChartView_(CQ_);
   CQChart *chart = CQChartView_getChart(view);
   CQChart_addLineSeries(chart, rickerSeries(1.0));
   CQChart_addLineSeries(chart, rickerSeries(2.0));
   CQChart_createDefaultAxes(chart);
   CQWidget *view_ = CQWidget_cast(view);
   CQWidget_setMinimumSize(view_, 800, 600);
   CQWidget_show(view_);
   return scope_leave_int(CQApplication_exec(app));
}
