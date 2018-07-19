// https://github.com/KubaO/stackoverflown/tree/master/questions/static-metatype-50703377
#include <QtCore>

// Interface

#if defined(QT_WIDGETS_LIB) && defined(Q_OS_MAC)
constexpr int StaticMetatypeId = QMetaType::User + 1;
#else
constexpr int StaticMetatypeId = QMetaType::User + 0;
#endif

#define MY_DECLARE_STATIC_METATYPE(TYPE, METATYPEID) \
   MY_DECLARE_STATIC_METATYPE_IMPL(TYPE, METATYPEID)

#define MY_DECLARE_STATIC_METATYPE_IMPL(TYPE, METATYPEID) \
    QT_BEGIN_NAMESPACE \
    template<> struct QMetaTypeId2<TYPE> \
    { \
        enum { Defined = 1, IsBuiltIn = false, MetaType = METATYPEID };   \
        static inline constexpr int qt_metatype_id() { return METATYPEID; } \
        static inline constexpr const char *static_name() { return #TYPE; } \
    }; \
    QT_END_NAMESPACE

template <typename T>
int registerStaticMetaType() {
   auto id = qRegisterMetaType<T>(QMetaTypeId2<T>::static_name(),
                                  reinterpret_cast<T*>(quintptr(-1)));
   Q_ASSERT(id == qMetaTypeId<T>());
   return id;
}

// Use

struct Foo {};
MY_DECLARE_STATIC_METATYPE(Foo, StaticMetatypeId + 0)

int main() {
   registerStaticMetaType<Foo>(); // StaticMetatypeId + 0
                                  // other registrations must follow in ascending order
   static_assert(qMetaTypeId<Foo>() == StaticMetatypeId+0, "try again :(");
}
