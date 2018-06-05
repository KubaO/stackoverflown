#include "qc_binding.h"
#include <QtCharts>
#include <climits>
#include <cstdarg>

// Scope

struct ScopeEntry {
   void *ptr;
   void (*deleter)(void*);
   bool operator==(void *ptr) const { return this->ptr == ptr; }
};

static QStack<QVector<ScopeEntry>> scope;

template <typename T, typename ...Args> static T *s_new(int option, Args &&...args) {
   T *obj = new T(std::forward<Args>(args)...);
   if (!(option & CQ_new))
      scope.top().append({obj, +[](void *p){ delete static_cast<T*>(p); }});
   return obj;
}

template <typename T> void s_delete(T *obj) {
   if (obj && !scope.isEmpty()) {
      auto &level = scope.top();
      auto it = std::find(level.rbegin(), level.rend(), obj);
      if (it != level.rend())
         level.erase((&*it));
   }
   delete obj;
}

extern "C" {

void scope_enter(void) {
   scope.push({});
}
static ScopeEntry scope_leave(const QVector<ScopeEntry> &level, void *ptr = nullptr) {
   ScopeEntry entry{};
   for (auto it = level.rbegin(); it != level.rend(); ++it)
      if (Q_LIKELY(it->ptr != ptr))
         it->deleter(it->ptr);
      else {
         Q_ASSERT(!entry.ptr);
         entry = *it;
      }
   return entry;
}
void scope_leave(void) {
   scope_leave(scope.pop());
}
int scope_leave_int(int i) {
   scope_leave();
   return i;
}
void *scope_leave_ptr(void *ptr) {
   auto entry = scope_leave(scope.pop(), ptr);
   if (entry.ptr) {
      Q_ASSERT(!scope.isEmpty());
      scope.top().append(entry);
   }
   return ptr;
}

} // extern "C"

// Type Helpers

template <typename T> struct TypeTag{};

struct CQVectorTypeId {
   void  (*construct)(void *);
   void  (*copy_construct)(void *, const void *);
   void  (*from_raw)(void*, const void *, size_t);
   void  (*resize)(void *, int);
   void *(*data_at)(void *, int);
   void  (*destruct)(void *);

   template <typename T> CQVectorTypeId(TypeTag<T>) :
      construct([](void *p){ new (p) QVector<T>(); }),
      copy_construct([](void *p, const void *src){ new (p) QVector<T>(*(const QVector<T>*)src); }),
      from_raw([](void *p, const void *raw, size_t n) {
      QTypedArrayData<T> *&data = *(QTypedArrayData<T>**)p;
         if (Q_LIKELY(n))
            data = QTypedArrayData<T>::fromRawData((const T*)raw, n);
         else
            data = QTypedArrayData<T>::sharedNull();
      }),
      resize([](void *p, int size){ ((QVector<T>*)p)->resize(size); }),
      data_at([](void *p, int i){ return (void*)(((QVector<T>*)p)->data()+i); }),
      destruct([](void *p){ ((QVector<T>*)p)->~QVector(); })
   {}
};

struct TypeId {
   // Type helpers for various uses
   int id;
   CQVectorTypeId *qVector = {};
};

struct CType_t : TypeId { using TypeId::TypeId; };

static QReadWriteLock typeIdsLock;
static QVector<TypeId> typeIds;

static const TypeId *getTypeId(int metaId) {
   Q_ASSERT(metaId > 0);
   QReadLocker read(&typeIdsLock);
   if (typeIds.size() > metaId)
      return &typeIds.at(metaId);
   read.unlock();
   QWriteLocker write(&typeIdsLock);
   typeIds.resize(std::max(typeIds.size(), metaId+1));
   typeIds[metaId].id = metaId;
   return &typeIds[metaId];
}

template <typename T> static const TypeId *getQVectorId() {
   TypeId *const typeId = const_cast<TypeId*>(getTypeId(qMetaTypeId<T>()));
   QReadLocker read(&typeIdsLock);
   if (typeId->qVector)
      return typeId;
   read.unlock();

   QScopedPointer<CQVectorTypeId> newId(new CQVectorTypeId(TypeTag<T>()));
   QWriteLocker write(&typeIdsLock);
   if (typeId->qVector)
      return typeId;
   typeId->qVector = newId.take();
   return typeId;
}

// QPointF

Q_DECLARE_METATYPE(QPointF)
static_assert(sizeof(QPointF) == 2*sizeof(double), "qreal must be a double");
const CType *CQPointF_type(void) { return static_cast<const CType*>(getQVectorId<QPointF>()); }
CQPointF *CQPointF_(void *d) { return (CQPointF*)d; }

// QVector

static_assert(sizeof(QVector<int>) == sizeof(void*), "QVector must be pointer-sized");
struct CQVector_t {
   struct { void * p; } vector;
   CQVectorTypeId *type;
   CQVector_t(CQVectorTypeId *type, const CQVector *src) : type(type) {
      if (src) type->copy_construct(&vector, src); else type->construct(&vector); }
   CQVector_t(CQVectorTypeId *type, const void *raw, size_t count) {
      type->from_raw(&vector, raw, count);
   }
   ~CQVector_t() { type->destruct(&vector); }
   template <typename T> inline QVector<T> *get() {
      Q_ASSERT(getQVectorId<T>()->qVector == type);
      return (QVector<T>*)&vector;
   }
   template <typename T> inline const QVector<T> *get() const {
      Q_ASSERT(getQVectorId<T>()->qVector == type);
      return (const QVector<T>*)&vector;
   }
};

extern "C" {

CQVector *CQVector_(int option, const CType *type, const CQVector *source) { return s_new<CQVector>(option, type->qVector, source); }
CQVector *CQVector_size_(int option, const CType *type, int size) {
   auto *v = s_new<CQVector>(option, type->qVector, nullptr);
   type->qVector->resize(&v->vector, size);
   return v;
}
CQVector *CQVector_from_raw(int option, const CType *type, const void *rawData, size_t n) {
   return s_new<CQVector>(option, type->qVector, rawData, n);
}
void CQVector_resize(CQVector *d, int size) { d->type->resize(&d->vector, size); }
void *CQVector_data_at(CQVector *d, int i) { return d->type->data_at(&d->vector, i); }
void *CQVector_typed_data_at(CQVector *d, const CType *type, int i) {
   return (d->type == type->qVector) ? d->type->data_at(&d->vector, i) : nullptr;
}
void CQVector_free(CQVector *d) { s_delete(d); }

// QString

struct CQString_t : QString {
   using QString::QString;
   CQString_t(QString &&str) : QString(std::move(str)) {}
   CQString_t(const QString &str) : QString(str) {}
};

CQString *CQString_(int option, const CQString *source) { return source ? s_new<CQString>(option, *source) : s_new<CQString>(option); }
CQString *CQString_utf8_(int option, const char *utf8) { return s_new<CQString>(option, QString::fromUtf8(utf8)); }
CQString *CQString_asprintf(int option, const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   auto *str = s_new<CQString>(option, QString::vasprintf(fmt, args));
   va_end(args);
   return str;
}
void qs_arg_dbl(CQString *d, double val, int fieldWidth) { *d = d->arg(val, fieldWidth); }
void CQstring_free(CQString *d) { s_delete(d); }

// QLineSeries

struct CQLineSeries_t : QLineSeries {};

CQLineSeries *CQLineSeries_(int option) { return s_new<CQLineSeries>(option); }
void CQLineSeries_setName(CQLineSeries *d, const CQString *name) { d->setName(*name); }
void CQLineSeries_setName_cstr(CQLineSeries *d, const char *name) { d->setName(QString::fromUtf8(name)); }
void CQLineSeries_replaceVector(CQLineSeries *d, const CQVector *data) { d->replace(*data->get<QPointF>()); }

// QChart

struct CQChart_t : QChart {};

void CQChart_addLineSeries(CQChart *d, CQLineSeries *series) { d->addSeries(series); }
void CQChart_createDefaultAxes(CQChart *d) { d->createDefaultAxes(); }

// QChartView

struct CQChartView_t : QChartView {};

CQChartView *CQChartView_(int option) { return s_new<CQChartView>(option); }
CQChart *CQChartView_getChart(CQChartView *d) { return static_cast<CQChart*>(d->chart()); }
void CQChartView_free(CQChartView *d) { s_delete(d); }

// QWidget

struct CQWidget_t : QWidget {};

CQWidget *CQWidget_cast(void *d) {
   return (CQWidget*)qobject_cast<QWidget*>((QObject*)d);
}
void CQWidget_setMinimumSize(CQWidget *d, int width, int height) { d->setMinimumSize(width, height); }
void CQWidget_show(CQWidget *d) { d->show(); }
void CQWidget_free(CQWidget *d) { s_delete(d); }

// QApplication

struct CQApplication_t : QApplication { using QApplication::QApplication; };

CQApplication *CQApplication_(int option, int *argc, char *argv[]) { return s_new<CQApplication>(option, *argc, argv); }
int CQApplication_exec(CQApplication *d) { return d->exec(); }
void CQApplication_free(CQApplication *d) { s_delete(d); }

} // extern "C"
