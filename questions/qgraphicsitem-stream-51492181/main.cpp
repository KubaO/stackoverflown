#include <QtWidgets>
#include <QtTest>
#include <type_traits>
#include <utility>

#if 0
/* Common polymorphic type machinery */

namespace detail {

template <typename T> struct CopyableFunctionHelperImpl {
   static void Destruct(void *t) { static_cast<T*>(t)->~T(); }
   static void *Construct(void *where, const void *t) {
      return t ? new (where) T(*static_cast<const T*>(t)) : new (where) T;
   }
   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(t); }
   static void Load(QDataStream &stream, void *t) { ds >> *static_cast<T*>(t); }
};

template <typename T> struct NonCopyableFunctionHelperImpl {
   static void Destruct(void *t) { static_cast<T*>(t)->~T(); }
   static void *Construct(void *where, const void *t) { return (!t) ? new (where) T : nullptr; }
   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(t); }
   static void Load(QDataStream &stream, void *t) { ds >> *static_cast<T*>(t); }
};

template <typename B, bool Enabled = false> struct PolymorphicTraits;

template <typename B> struct PolymorphicTraits<B, true> {
   // By default, discriminate types by their metatype Id
   using discriminator_type = int;
   template <typename T> static discriminator_type type() { return qMetaTypeId<T>(); }
   template <typename T> static void *create(void *copy) { return QMetaType::create(type<T>(), copy); }
   template <typename T> static discriminator_type registerDiscriminator() { return type<T>(); }
};

}

#define DECLARE_POLYMORPHIC_METATYPE(BASE_TYPE, TYPE) DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)
#define DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)           \
   QT_BEGIN_NAMESPACE                                                \
   template <> struct QtMetaTypePrivate::QMetaTypeFunctionHelper     \
   <typename std::enable_if<std::is_copy_constructible<TYPE>::value, TYPE>::type, true> : \
   detail::CopyableFunctionHelperImpl<TYPE> {};                   \
   template <> struct QtMetaTypePrivate::QMetaTypeFunctionHelper     \
   <typename std::enable_if<!std::is_copy_constructible<TYPE>::value, TYPE>::type, true> : \
   detail::NonCopyableFunctionHelperImpl<TYPE> {};                \
   template <>                                                       \
   int qRegisterMetaType(const char *typeName, TYPE *dummy,          \
   typename QtPrivate::MetaTypeDefinedHelper<T, QMetaTypeId2<T>::Defined && !QMetaTypeId2<T>::IsBuiltIn>::DefinedType defined) \
{ \
   QT_PREPEND_NAMESPACE(QByteArray) normalizedTypeName = QMetaObject::normalizedType(typeName); \
   detail::PolymorphicTraits<BASE_TYPE>::registerDiscriminator<TYPE>();   \
   return qRegisterNormalizedMetaType<TYPE>(normalizedTypeName, dummy, defined); \
   } \
   template <> struct QMetaTypeId<TYPE> {                            \                                                                  \
   enum { Defined = 1 };                                          \
   static int qt_metatype_id() {                                  \
   static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
   if (const int id = metatype_id.loadAcquire())               \
   return id;                                                  \
   const int newId = qRegisterMetaType<TYPE>(#TYPE,            \
   reinterpret_cast<TYPE*>(quintptr(-1)));                  \
   metatype_id.storeRelease(newId);                            \
   return newId;                                               \
   }                                                              \
   };                                                                \
   QT_END_NAMESPACE



#define DEFINE_POLYMORPHIC_METATYPE(TYPE) DEFINE_POLYMORPHIC_METATYPE_IMPL(TYPE)
#define DEFINE_POLYMORPHIC_METATYPE_IMPL(TYPE) \
   static

Q_GLOBAL_STATIC(QReadWriteLock, aa)


template <typename T> bool checkTypeEnum() {
   auto const &en = QMetaEnum::fromType<T>();
   for (int i = 0; i < en.keyCount(); i++)
      if (QMetaType::type(en.key(i)) == QMetaType::UnknownType)
         return false;
   return true;
}

template <typename T> int typeIdToEnum(int typeId) {
   auto const &en = QMetaEnum::fromType<T>();
   return en.keyToValue(QMetaType::typeName(typeId));
}

template <typename T> int enumToTypeId(int enumVal) {
   auto const &en = QMetaEnum::fromType<T>();
   auto *name = en.valueToKey(enumVal);
   return name ? QMetaType::type(name) : QMetaType::UnknownType;
}

#define DECLARE_GRAPHICS_METAITEM(TYPE) DECLARE_GRAPHICS_METAITEM_IMPL(TYPE)
#define DECLARE_GRAPHICS_METAITEM_IMPL(TYPE)                            \
   QT_BEGIN_NAMESPACE                                                   \
   template <>                                                          \
   struct QtMetaTypePrivate::QMetaTypeFunctionHelper<TYPE, true> :      \
   QGraphicsItemHelper<TYPE> {};                                     \
   template <>                                                          \
   struct QMetaTypeId<TYPE>                                             \
{                                                                    \
   enum { Defined = 1 };                                            \


DECLARE_GRAPHICS_METAITEM(QGraphicsEllipseItem)
#endif


template <typename T, typename B> struct NonCopyableFunctionHelper {
   using base_type = B;
   static void Destruct(void *t) { static_cast<T*>(t)->~T(); }
   static void *Construct(void *where, const void *t) { return (!t) ? new (where) T : nullptr; }
   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(t); }
   static void Load(QDataStream &ds, void *t) { ds >> *static_cast<T*>(t); }
};

#define DECLARE_POLYMORPHIC_METATYPE(BASE_TYPE, TYPE) DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)
#define DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)                          \
   QT_BEGIN_NAMESPACE                                                               \
   template <> struct QtMetaTypePrivate::QMetaTypeFunctionHelper                    \
   <typename std::enable_if<!std::is_copy_constructible<TYPE>::value, TYPE>::type, true> : \
      detail::NonCopyableFunctionHelperImpl<TYPE, BASE_TYPE> {};                    \
   QT_END_NAMESPACE                                                                 \
   Q_DECLARE_METATYPE_IMPL(TYPE)

//DECLARE_POLYMORPHIC_METATYPE(QGraphicsItem, QGraphicsEllipseItem)


/*Interface*/
template <> struct QtMetaTypePrivate::QMetaTypeFunctionHelper
      <typename std::enable_if<!std::is_copy_constructible<QGraphicsEllipseItem>::value, QGraphicsEllipseItem>::type, true> :
      NonCopyableFunctionHelper<QGraphicsEllipseItem, QGraphicsItem> {};
Q_DECLARE_METATYPE(QGraphicsEllipseItem)

template <typename T, typename Enable = std::false_type> struct BaseTraits;

template <> struct BaseTraits<QGraphicsItem, std::true_type> {
   using base_type = QGraphicsItem;
   struct pair_type { int itemType; int typeId; };
   template <typename T>
   static typename std::enable_if<std::is_base_of<base_type, T>::value>::type registerType() {
      qRegisterMetaTypeStreamOperators<T>();
      registerMapping(qMetaTypeId<T>(), T::type());
   }
   static pair_type registerMapping(int typeId, int d);
};

QDataStream &operator<<(QDataStream &, QGraphicsItem *g);
QDataStream &operator>>(QDataStream &, QGraphicsItem *&g);

/*Implementation*/

QDataStream &operator<<(QDataStream &ds, const QGraphicsItem *g) {
   auto mapping = BaseTraits<QGraphicsItem, std::true_type>::registerMapping(QMetaType::UnknownType, g->type());
   QMetaType::save(ds, mapping.typeId, g);
   return ds;
}

QDataStream &operator>>(QDataStream &, QGraphicsItem *&g) {
   int type = 0;
   auto read = ds.device()->peek(reinterpret_cast<char*>(&type), sizeof(type));
   if (read != sizeof(type)) {
      ds.setStatus(QDataStream::ReadPastEnd);
      return ds;
   }
   auto mapping = BaseTraits<QGraphicsItem, std::true_type>::registerMapping(type, 0);
   const QMetaType mt(mapping.typeId);
   g = mt.create();
   if (g) mt.load()
   QMetaClassIn
}


class ItemStream {
   Q_GADGET
   QDataStream &ds;
   QGraphicsItem &item;
public:
   enum Type {
      QGraphicsEllipseItem = QGraphicsEllipseItem::Type,
      QGraphicsPathItem = QGraphicsPathItem::Type,
      QGraphicsPolygonItem = QGraphicsPolygonItem::Type,
      QGraphicsRectItem = QGraphicsRectItem::Type,
      QGraphicsSimpleTextItem = QGraphicsSimpleTextItem::Type,
      QGraphicsLineItem = QGraphicsLineItem::Type,
      QGraphicsPixmapItem = QGraphicsPixmapItem::Type
   };
   Q_ENUM(Type)
   ItemStream(QDataStream &ds, class QGraphicsItem &item) : ds(ds), item(item) {}
   template <typename C, typename T> ItemStream &operator>>(void (C::*set)(T)) {
      using decayed_type = typename std::decay<T>::type;
      using value_type = typename std::conditional<std::is_enum<decayed_type>::value,
      std::underlying_type<decayed_type>, std::decay<T>>::type::type;
      value_type value;
      ds >> value;
      (static_cast<C&>(item).*set)(static_cast<T>(value));
      return *this;
   }
};

QDataStream &operator<<(QDataStream &out, const QGraphicsItem &g) {
   out << g.type()
       << g.pos()
       << g.scale()
       << g.rotation()
       << g.transform()
          //<< g.transformations()
       << g.transformOriginPoint()
       << g.flags()
       << g.isEnabled()
       << g.isSelected()
       << g.zValue();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   int type;
   QTransform transform;
   in >> type;
   Q_ASSERT(g.type() == type);
   ItemStream iin(in, g);
   iin >> &QGI::setPos
         >> &QGI::setScale
         >> &QGI::setRotation;
   in >> transform;
   iin //>> &QGI::setTransformations
         >> &QGI::setTransformOriginPoint
         >> &QGI::setFlags
         >> &QGI::setEnabled
         >> &QGI::setSelected
         >> &QGI::setZValue;
   g.setTransform(transform);
   return in;
}

QDataStream &operator<<(QDataStream &out, const QAbstractGraphicsShapeItem &g){
   out << static_cast<const QGraphicsItem&>(g);
   out << g.pen() << g.brush();
   return out;
}

QDataStream &operator>>(QDataStream &in, QAbstractGraphicsShapeItem &g){
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QGraphicsItem &>(g);
   ItemStream(in,g) >> &QGI::setPen >> &QGI::setBrush;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsEllipseItem &g){
   out << dynamic_cast<const QAbstractGraphicsShapeItem &>(g);
   out << g.rect() << g.startAngle() << g.spanAngle();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsEllipseItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QAbstractGraphicsShapeItem &>(g);
   ItemStream(in, g) >> &QGI::setRect >> &QGI::setStartAngle >> &QGI::setSpanAngle;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsPathItem &g) {
   out << static_cast<const QAbstractGraphicsShapeItem &>(g);
   out << g.path();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsPathItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QAbstractGraphicsShapeItem &>(g);
   ItemStream(in, g) >> &QGI::setPath;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsPolygonItem &g) {
   out << static_cast<const QAbstractGraphicsShapeItem &>(g);
   out << g.polygon()<< g.fillRule();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsPolygonItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QAbstractGraphicsShapeItem &>(g);
   ItemStream(in, g) >> &QGI::setPolygon >> &QGI::setFillRule;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsRectItem &g) {
   out << static_cast<const QAbstractGraphicsShapeItem &>(g);
   out << g.rect();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsRectItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QAbstractGraphicsShapeItem &>(g);
   ItemStream(in,g) >> &QGI::setRect;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsSimpleTextItem &g) {
   out << static_cast<const QAbstractGraphicsShapeItem &>(g);
   out << g.text() << g.font();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsSimpleTextItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QAbstractGraphicsShapeItem&>(g);
   ItemStream(in, g) >> &QGI::setText >> &QGI::setFont;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsLineItem &g) {
   out << static_cast<const QGraphicsItem &>(g);
   out << g.pen() << g.line();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsLineItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QGraphicsItem&>(g);
   ItemStream(in, g) >> &QGI::setPen >> &QGI::setLine;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsPixmapItem &g) {
   out << static_cast<const QGraphicsItem&>(g);
   out << g.pixmap() << g.offset() << g.transformationMode() << g.shapeMode();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsPixmapItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QGraphicsItem&>(g);
   ItemStream(in, g) >> &QGI::setPixmap >> &QGI::setOffset
                     >> &QGI::setTransformationMode >> &QGI::setShapeMode;
   return in;
}

#if 0
static void saveItems(QList<QGraphicsItem *> items, QDataStream & out){
   for(QGraphicsItem *item : items){
      out << g.type();
      switch (g.type()) {
      case QGraphicsLineItem::Type:
         out << dynamic_cast<QGraphicsLineItem *>(item);
         break;
      case QGraphicsSimpleTextItem::Type:
         out << dynamic_cast<QGraphicsSimpleTextItem *>(item);
         break;
      case QGraphicsRectItem::Type:
         out << dynamic_cast<QGraphicsRectItem *>(item);
         break;
      case QGraphicsPolygonItem::Type:
         out << dynamic_cast<QGraphicsPolygonItem *>(item);
         break;
      case QGraphicsPathItem::Type:
         out << dynamic_cast<QGraphicsPathItem *>(item);
         break;
      case QGraphicsPixmapItem::Type:
         out << dynamic_cast<QGraphicsPixmapItem *>(item);
         break;
      }
   }
}

static QList<QGraphicsItem *> readItems(QDataStream & in){
   QList<QGraphicsItem *> items;
   int type;
   while (!in.atEnd()) {
      in >> type;
      switch (type) {
      case QGraphicsLineItem::Type: {
         QGraphicsLineItem *item = new QGraphicsLineItem;
         in >> item;
         items << item;
         break;
      }
      case QGraphicsSimpleTextItem::Type:{
         QGraphicsSimpleTextItem *item = new QGraphicsSimpleTextItem;
         in >> item;
         items << item;
         break;
      }
      case QGraphicsRectItem::Type:{
         QGraphicsRectItem *item = new QGraphicsRectItem;
         in >> item;
         items << item;
         break;
      }
      case QGraphicsPolygonItem::Type:{
         QGraphicsPolygonItem *item = new QGraphicsPolygonItem;
         in >> item;
         items << item;
         break;
      }
      case QGraphicsPathItem::Type:{
         QGraphicsPathItem *item = new QGraphicsPathItem;
         in >> item;
         items << item;
         break;
      }
      case QGraphicsPixmapItem::Type:{
         QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
         in >> item;
         items << item;
         break;
      }
      }
   }
   return  items;
}
#endif

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   return a.exec();
}
#include "main.moc"
