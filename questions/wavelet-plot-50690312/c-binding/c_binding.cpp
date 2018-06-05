#include "c_binding.h"
#include <QtCharts>
#include <climits>

static_assert(sizeof(QPointF) == 2*sizeof(double), "qreal must be a double");

struct ScopeEntry {
   void *ptr;
   void (*deleter)(void*);
};

static QStack<QVector<ScopeEntry>> scope;

template <typename T, typename ...Args> static T *s_new(int option, Args &&...args) {
   T *obj = new T(std::forward<Args>(args)...);
   if (option & CQ_scoped)
      scope.top().append({obj, +[](void *p){ delete static_cast<T*>(p); }});
   return obj;
}

extern "C" {

void scope_enter(void) {
   scope.push({});
}
static void scope_leave(const QVector<ScopeEntry> &level, void *ptr = nullptr) {
   for (auto &entry : level)
      if (Q_LIKELY(entry.ptr != ptr))
         entry.deleter(entry.ptr);
}
void scope_leave(void) {
   scope_leave(scope.pop());
}
int scope_leave_int(int i) {
   scope_leave();
   return i;
}
void *scope_leave_ptr(void *ptr) {
   scope_leave(scope.pop(), ptr);
   return ptr;
}

struct CQVector_QPointF_t : QVector<QPointF> {};

CQVector_QPointF *CQVector_QPointF_(int option) { return s_new<CQVector_QPointF>(option); }
void CQVector_QPointF_resize(CQVector_QPointF *d, int size) { d->resize(size); }
double *CQVector_QPointF_x(CQVector_QPointF *d, int i) { return ((double*)&(*d)[i])+0; }
double *CQVector_QPointF_y(CQVector_QPointF *d, int i) { return ((double*)&(*d)[i])+1; }
void CQVector_QPointF_free(CQVector_QPointF *d) { delete d; }

struct CQLineSeries_t : QLineSeries {};

CQLineSeries *CQLineSeries_(int option) { return s_new<CQLineSeries>(option); }
void CQLineSeries_setName(CQLineSeries *d, const char *name) { d->setName(QString::fromUtf8(name)); }
void CQLineSeries_replaceVector(CQLineSeries *d, const CQVector_QPointF *data) { d->replace(*data); }

struct CQChart_t : QChart {};

void CQChart_addLineSeries(CQChart *d, CQLineSeries *series) { d->addSeries(series); }
void CQChart_createDefaultAxes(CQChart *d) { d->createDefaultAxes(); }

struct CQChartView_t : QChartView {};

CQChartView *CQChartView_(int option) { return s_new<CQChartView>(option); }
CQChart *CQChartView_getChart(CQChartView *d) { return static_cast<CQChart*>(d->chart()); }
void CQChartView_free(CQChartView *d) { delete d; }

struct CQWidget_t : QWidget {};

CQWidget *CQWidget_from(void *d) {
   return (CQWidget*)qobject_cast<QWidget*>((QObject*)d);
}
void CQWidget_setMinimumSize(CQWidget *d, int width, int height) { d->setMinimumSize(width, height); }
void CQWidget_show(CQWidget *d) { d->show(); }

struct CQApplication_t : QApplication { using QApplication::QApplication; };

CQApplication *CQApplication_(int option, int *argc, char *argv[]) { return s_new<CQApplication>(option, *argc, argv); }
int CQApplication_exec(CQApplication *d) { return d->exec(); }
void CQApplication_free(CQApplication *d) { delete d; }

} // extern "C"
