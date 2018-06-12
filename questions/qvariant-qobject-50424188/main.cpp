// https://github.com/KubaO/stackoverflown/tree/master/questions/qvariant-qobject-50424188
#include <QtWidgets>
#include <memory>

enum ExtraTypeFlag {
   PolymorphicInterface = 0x1000
}

#define DECLARE_METAINTERFACE(Type) \
   namespace QtMetaTypePrivate { \
   template <> struct QMetaTypeFunctionHelper<Type, true> : QMetaTypeFunctionHelper<void> { \
   static void Destruct(void *) { \
   qFatal("Cannot destruct an interface instance "#Type); \
   } \
   static void *Construct(void *, const void *) { \
   qFatal("Cannot construct an interface instance "#Type); \
   } \
   }; } \
   Q_DECLARE_METATYPE(Type)

namespace detail {
   template<typename T> struct MetaTypeTypeFlags
{
      enum { Flags = (QTypeInfoQuery<T>::isRelocatable ? QMetaType::MovableType : 0)
             | (QTypeInfo<T>::isComplex ? QMetaType::NeedsConstruction : 0)
             | (QTypeInfo<T>::isComplex ? QMetaType::NeedsDestruction : 0)
             | (IsPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::PointerToQObject : 0)
             | (IsSharedPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::SharedPointerToQObject : 0)
             | (IsWeakPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::WeakPointerToQObject : 0)
             | (IsTrackingPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::TrackingPointerToQObject : 0)
             | (std::is_enum<T>::value ? QMetaType::IsEnumeration : 0)
             | (IsGadgetHelper<T>::Value ? QMetaType::IsGadget : 0)
             | (IsPointerToGadgetHelper<T>::Value ? QMetaType::PointerToGadget : 0)
      };
   };

#define DECLARE_POLYMORPHIC_METATYPE(Type) \
   Q_DECLARE_METATYPE(Type) \
   template <> bool QVariant::canConvert() const \


   template<typename T>
   struct QMetaTypeTypeFlags
{
      enum { Flags = (QTypeInfoQuery<T>::isRelocatable ? QMetaType::MovableType : 0)
             | (QTypeInfo<T>::isComplex ? QMetaType::NeedsConstruction : 0)
             | (QTypeInfo<T>::isComplex ? QMetaType::NeedsDestruction : 0)
             | (IsPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::PointerToQObject : 0)
             | (IsSharedPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::SharedPointerToQObject : 0)
             | (IsWeakPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::WeakPointerToQObject : 0)
             | (IsTrackingPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::TrackingPointerToQObject : 0)
             | (std::is_enum<T>::value ? QMetaType::IsEnumeration : 0)
             | (IsGadgetHelper<T>::Value ? QMetaType::IsGadget : 0)
             | (IsPointerToGadgetHelper<T>::Value ? QMetaType::PointerToGadget : 0)
      };
   };


   template <>

   template <>                                                         \
   struct QMetaTypeId< TYPE >                                          \
{                                                                   \
      enum { Defined = 1 };                                           \
      static int qt_metatype_id()                                     \
      {                                                           \
         static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
         if (const int id = metatype_id.loadAcquire())           \
         return id;                                          \
         const int newId = qRegisterMetaType< TYPE >(#TYPE,      \
         reinterpret_cast< TYPE *>(quintptr(-1))); \
         metatype_id.storeRelease(newId);                        \
         return newId;                                           \
      }                                                           \
   };                                                                  \

   class ITest {
      public:
      virtual QString test() = 0;
      virtual ~ITest() = default;
   };
   //DECLARE_METAINTERFACE(ITest)

   class Class : public QObject {
      Q_OBJECT
      public:
      using QObject::QObject;
      QString data = "Object";
   };


   class TestClass : public QObject, public ITest {
      Q_OBJECT
      public:
      using QObject::QObject;
      QString data = "TestClass";

      QString test() override {
         return data;
      }
   };

   class Gadget {
      Q_GADGET
      static QString mark(const QString &o) { return QStringLiteral("%1+").arg(o); }
      public:
      Gadget() = default;
      Gadget(const Gadget &o) : data(mark(o.data)) {}
      Gadget &operator=(const Gadget &o) & { data = mark(o.data); return *this; }
      QString data = "Gadget";
      virtual ~Gadget() = default; // must be polymorphic!
   };

   template<> const ITest *qvariant_cast(const QVariant &v) {
      if (v.canConvert<QObject*>()) {
         auto *o = v.value<QObject*>();
         return dynamic_cast<const ITest*>(o);
      }
      else if (v.canConvert<ITest>())
      return static_cast<const ITest*>(v.data());
      return {};
   }

   template<> ITest *qvariant_cast(QVariant &v) {
      if (v.canConvert<QObject*>()) {
         auto *o = v.value<QObject*>();
         return dynamic_cast<ITest*>(o);
      }
      else if (v.canConvert<ITest>())
      return static_cast<ITest*>(const_cast<void*>(v.data()));
      return {};
   }

   class DataContainingClass: public ITest {
      QMap<QString,QVariant> data;
      public:
      QString test() override {
         QString result;
         auto add = [&](const QString &s) { result = QStringLiteral("%1\n%2").arg(result).arg(s); };
         for (auto it = data.begin(); it != data.end(); it++) {
            auto *t = qvariant_cast<ITest*>(it.value());
            if (t)
            add(t->test());
         }
         return result;
      }
   };


   int main(int argc, char *argv[])
{
      QApplication a(argc, argv);

      // Class in QVariant
      auto *c = new Class;
      auto vc = QVariant::fromValue(c);

      Class *c2 = vc.value<Class*>();
      Q_ASSERT(c2 == c);
      auto *objc = *reinterpret_cast<QObject**>(vc.data());
      Q_ASSERT(objc == c); // did we get the same object?
      Q_ASSERT(dynamic_cast<ITest*>(objc) == 0);

      // Object using correct base class in QVariant
      auto *t = new TestClass;
      auto vt = QVariant::fromValue(t);

      auto *t2 = vt.value<TestClass*>();
      Q_ASSERT(t2 == t);
      auto *objT = *reinterpret_cast<QObject**>(vt.data());
      Q_ASSERT(objT == t);
      auto *it = dynamic_cast<ITest*>(objT);
      Q_ASSERT(it);
      Q_ASSERT(it->test() == t2->test());

      // Q_Gadget in QVariant
      Gadget g;
      auto vg = QVariant::fromValue(g);

      Q_ASSERT(vg.canConvert<Gadget>());
      auto g2 = vg.value<Gadget>();
      Q_ASSERT(dynamic_cast<QObject*>(&g2) == 0);

      // Some other value in QVariant
      QVariant o = 4;
      Q_ASSERT(!o.canConvert<QObject*>());
   }

#include "main.moc"
