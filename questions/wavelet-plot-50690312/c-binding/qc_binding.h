#ifndef QC_BINDING_H
#define QC_BINDING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { CQ_scoped = 0, CQ_ = CQ_scoped, CQ_new = 0x4000000 };

void scope_enter(void);
void scope_leave(void);
int scope_leave_int(int);
void *scope_leave_ptr(void *);

typedef struct CType_t CType;
const CType *CQPointF_type(void);
typedef struct CQPointF_t { double x; double y; } CQPointF;
CQPointF *CQPointF_(void *d);

typedef struct CQVector_t CQVector;
CQVector *CQVector_(int option, const CType *, const CQVector *source);
CQVector *CQVector_size_(int option, const CType *, int size);
CQVector *CQVector_from_raw(int option, const CType *, const void *rawData, size_t n);
void CQVector_resize(CQVector *d, int size);
void *CQVector_data_at(CQVector *d, int i);
void *CQVector_typed_data_at(CQVector *d, const CType *, int i);
void CQVector_free(CQVector *d);

typedef struct CQString_t CQString;
CQString *CQString_(int option, const CQString *source);
CQString *CQString_utf8_(int option, const char *utf8);
CQString *CQString_asprintf(int option, const char *fmt, ...);
void qs_arg_dbl(CQString *d, double val, int fieldWidth);
void CQstring_free(CQString *d);

typedef struct CQLineSeries_t CQLineSeries;
CQLineSeries *CQLineSeries_(int option);
void CQLineSeries_setName(CQLineSeries *d, const CQString *name);
void CQLineSeries_setName_cstr(CQLineSeries *d, const char *name);
void CQLineSeries_replaceVector(CQLineSeries *d, const CQVector *data);

typedef struct CQChart_t CQChart;
void CQChart_addLineSeries(CQChart *d, CQLineSeries *series);
void CQChart_createDefaultAxes(CQChart *d);

typedef struct CQChartView_t CQChartView;
CQChartView *CQChartView_(int option);
CQChart *CQChartView_getChart(CQChartView *d);
void CQChartView_free(CQChartView *d);

typedef struct CQWidget_t CQWidget;
CQWidget *CQWidget_cast(void *d);
void CQWidget_setMinimumSize(CQWidget *d, int width, int height);
void CQWidget_show(CQWidget *d);
void CQWidget_free(CQWidget *d);

typedef struct CQApplication_t CQApplication;
CQApplication *CQApplication_(int option, int *argc, char *argv[]);
int CQApplication_exec(CQApplication *d);
void CQApplication_free(CQApplication *d);

#ifdef __cplusplus
}
#endif

#endif // QC_BINDING_H
