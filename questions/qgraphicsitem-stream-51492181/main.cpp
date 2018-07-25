#include <QtWidgets>
#include <QtTest>
#include <type_traits>

namespace detail {
template <typename T> struct MetatypeFunctionHelperImpl {
   static void Destruct(void *t) { static_cast<T*>(t)->~T(); }
   static void *Construct(void *where, const void *t) {
      if (t)
         return new (where) T(*static_cast<const T*>(t));
      return new (where) T;
   }
   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(t); }
   static void Load(QDataStream &stream, void *t) { ds >> *static_cast<T*>(t); }
};
template <typename B, typename T> struct BaseMetatypeFunctionhelper<B, T> :
      MetatypeFunctionHelperImpl<T> {};
}

#define DECLARE_CUSTOM_METATYPE(BASE_TYPE, TYPE) DECLARE_CUSTOM_METATYPE_IMPL(BASE_TYPE, TYPE)

#define DECLARE_CUSTOM_METATYPE_IMPL(BASE_TYPE, TYPE) \
   QT_BEGIN_NAMESPACE \
   template <> \
   struct QtMetaTypePrivate::QMetaTypeFunctionHelper<TYPE, true> : \
   BaseMetatypeFunctionhelper<BASE_TYPE, TYPE> {}; \


Q_GLOBAL_STATIC(QReadWriteLock, aa)

                template <typename T> struct QGraphicsItemHelper {
                   static void Destruct(void *t) { static_cast<T*>(t)->~T(); }
                   static void *Construct(void *where, const void *t) {
                      Q_ASSERT(!t);
                      return new (where) T;
                   }
                   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(t); }
                   static void Load(QDataStream &ds, void *t) { ds >> *static_cast<T*>(t); }
                };

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
   static int qt_metatype_id()                                      \
{                                                            \
   static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
   if (const int id = metatype_id.loadAcquire())            \
   return id;                                           \
   const int newId = qRegisterMetaType< TYPE >(#TYPE,       \
   reinterpret_cast< TYPE *>(quintptr(-1)));  \
   metatype_id.storeRelease(newId);                         \
   return newId;                                            \
   }                                                            \
   };                                                                   \
   QT_END_NAMESPACE

DECLARE_GRAPHICS_METAITEM(QGraphicsEllipseItem)

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
