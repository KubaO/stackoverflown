#ifndef C_BINDING_H
#define C_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

enum { CQ_scoped = 0, CQ_ = CQ_scoped, CQ_new = 0x4000000 };

void scope_enter(void);
void scope_leave(void);
int scope_leave_int(int);
void *scope_leave_ptr(void *);

typedef struct CQVector_QPointF_t CQVector_QPointF;
CQVector_QPointF *CQVector_QPointF_(int option);
void CQVector_QPointF_resize(CQVector_QPointF *d, int size);
double *CQVector_QPointF_x(CQVector_QPointF *d, int i);
double *CQVector_QPointF_y(CQVector_QPointF *d, int i);
void CQVector_QPointF_free(CQVector_QPointF *d);

typedef struct CQLineSeries_t CQLineSeries;
CQLineSeries *CQLineSeries_(int option);
void CQLineSeries_setName(CQLineSeries *d, const char *name);
void CQLineSeries_replaceVector(CQLineSeries *d, const CQVector_QPointF *data);

typedef struct CQChart_t CQChart;
void CQChart_addLineSeries(CQChart *d, CQLineSeries *series);
void CQChart_createDefaultAxes(CQChart *d);

typedef struct CQChartView_t CQChartView;
CQChartView *CQChartView_(int option);
CQChart *CQChartView_getChart(CQChartView *d);
void CQChartView_free(CQChartView *d);

typedef struct CQWidget_t CQWidget;
CQWidget *CQWidget_from(void *d);
void CQWidget_setMinimumSize(CQWidget *d, int width, int height);
void CQWidget_show(CQWidget *d);

typedef struct CQApplication_t CQApplication;
CQApplication *CQApplication_(int option, int *argc, char *argv[]);
int CQApplication_exec(CQApplication *d);
void CQApplication_free(CQApplication *d);

#ifdef __cplusplus
}
#endif

#endif // C_BINDING_H
