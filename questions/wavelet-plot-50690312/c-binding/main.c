#include "c_binding.h"
#include "math.h"
#include "stddef.h"
#include "stdio.h"

const double pi = 3.14159265358979323846;

CQVector_QPointF *ricker(double f, double length, double dt) {
   scope_enter();
   CQVector_QPointF *w = CQVector_QPointF_(CQ_);
   size_t N = (length - dt/2.0)/dt;
   CQVector_QPointF_resize(w, N);
   for (size_t i = 0; i < N; ++i) {
      double t = -length/2 + i*dt;
      double *const w_xy = CQVector_QPointF_x(w, i);
      w_xy[0] = t;
      w_xy[1] = (1.0 - 2*pi*pi*f*f*t*t) * exp(-pi*pi*f*f*t*t);
   }
   return scope_leave_ptr(w);
}

CQLineSeries *rickerSeries(double f) {
   scope_enter();
   char buf[128];
   snprintf(buf, sizeof(buf)-1, "Ricker Wavelet for f=%.2f", f);
   CQLineSeries *series = CQLineSeries_(CQ_);
   CQLineSeries_setName(series, buf);
   CQVector_QPointF *vector = ricker(f, 2.0, 0.001);
   CQLineSeries_replaceVector(series, vector);
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
   CQWidget *view_ = CQWidget_from(view);
   CQWidget_setMinimumSize(view_, 800, 600);
   CQWidget_show(view_);
   return scope_leave_int(CQApplication_exec(app));
}
