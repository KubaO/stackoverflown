// https://github.com/KubaO/stackoverflown/tree/master/questions/qgraphicsitem-stream-51492181
// Interface
#include <QDataStream>
#include <QGraphicsItem>
#include <QGraphicsTransform>
#include <QMetaType>
#include <type_traits>
#include <algorithm>

template <class B, class T> struct NonConstructibleFunctionHelper {
   static void *Construct(void *, const void *) { return {}; }
   static void Destruct(void *t) { static_cast<T*>(static_cast<B*>(t))->~T(); }
   static void Save(QDataStream &ds, const void *t) { ds << *static_cast<const T*>(static_cast<const B*>(t)); }
   static void Load(QDataStream &ds, void *t) { ds >> *static_cast<T*>(static_cast<B*>(t)); }
};

template <class B, class T> struct NonCopyableFunctionHelper : NonConstructibleFunctionHelper<B, T> {
   static void *Construct(void *where, const void *t) { return (!t) ? static_cast<B*>(new (where) T) : nullptr; }
};

#define DECLARE_POLYMORPHIC_METATYPE(BASE_TYPE, TYPE) DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)
#define DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)        \
   QT_BEGIN_NAMESPACE namespace QtMetaTypePrivate {               \
   template <> struct QMetaTypeFunctionHelper<TYPE> :             \
      std::conditional<std::is_copy_constructible<TYPE>::value, void, \
      std::conditional<std::is_default_constructible<TYPE>::value, NonCopyableFunctionHelper<BASE_TYPE, TYPE>, \
      NonConstructibleFunctionHelper<BASE_TYPE, TYPE>             \
      >::type >::type {};                                         \
   QT_END_NAMESPACE }                                             \
   Q_DECLARE_METATYPE_IMPL(TYPE)
#define DECLARE_POLYSTREAMING_METATYPE(BASE_TYPE, TYPE) DECLARE_POLYSTREAMING_METATYPE_IMPL(BASE_TYPE, TYPE)
#define DECLARE_POLYSTREAMING_METATYPE_IMPL(BASE_TYPE, TYPE)      \
   DECLARE_POLYMORPHIC_METATYPE_IMPL(BASE_TYPE, TYPE)             \
   QDataStream &operator<<(QDataStream &, const TYPE &);          \
   QDataStream &operator>>(QDataStream &, TYPE &);

#define DECLARE_GRAPHICSITEM_METATYPE(TYPE) DECLARE_POLYSTREAMING_METATYPE_IMPL(QGraphicsItem, TYPE)

QDataStream &operator<<(QDataStream &, const QList<QGraphicsItem*> &);

void saveProperties(QDataStream &, const QObject *, const QByteArrayList &excluded = {});
void loadProperties(QDataStream &, QObject *);

QDataStream &operator<<(QDataStream &, const QGraphicsTransform *);
QDataStream &operator>>(QDataStream &, QGraphicsTransform *&);

void registerMapping(int typeId, int itemType);

template <typename T>
typename std::enable_if<std::is_base_of<QGraphicsItem, T>::value>::type registerMapping() {
   qRegisterMetaTypeStreamOperators<T>();
   registerMapping(qMetaTypeId<T>(), T::Type);
}

QDataStream &operator<<(QDataStream &, const QGraphicsItem *);
QDataStream &operator>>(QDataStream &, QGraphicsItem *&);

DECLARE_POLYMORPHIC_METATYPE(QGraphicsTransform, QGraphicsRotation)
DECLARE_POLYMORPHIC_METATYPE(QGraphicsTransform, QGraphicsScale)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsItem)
DECLARE_GRAPHICSITEM_METATYPE(QAbstractGraphicsShapeItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsItemGroup)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsLineItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsObject)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsPixmapItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsEllipseItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsPathItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsPolygonItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsRectItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsSimpleTextItem)
DECLARE_GRAPHICSITEM_METATYPE(QGraphicsTextItem)

class ItemStream {
   QDataStream &ds;
   QGraphicsItem &item;
public:
   using Version = qint8;
   static constexpr Version CurVersion = 1;
   enum { VersionKey = 0x9000 };
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
   static Version formatOf(const QGraphicsItem &item) {
      auto version = item.data(VersionKey);
      return Version(version.isValid() ? version.value<int>() : 0);
   }
   static void setVersion(QGraphicsItem &item, Version version) {
      item.setData(VersionKey, version);
   }
};

// Test
#include <QtWidgets>

QPixmap makePixmap() {
   QPixmap pix(100, 50);
   pix.fill(Qt::transparent);
   QPainter p(&pix);
   p.setPen(Qt::darkCyan);
   p.setFont({"helvetica,arial", 15});
   p.drawText(pix.rect(), Qt::AlignCenter, "Hello!");
   return pix;
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget ui;
   QGridLayout layout(&ui);
   QGraphicsScene scene;
   QGraphicsView view(&scene);
   QPushButton save("Save");
   QPushButton load("Load");
   layout.addWidget(&view, 0, 0, 1, 2);
   layout.addWidget(&load, 1, 0);
   layout.addWidget(&save, 1, 1);

   auto *eitem = scene.addEllipse(QRect(10, 10, 80, 50), QPen(Qt::green), QBrush(Qt::black));
   auto *xform = new QGraphicsRotation;
   xform->setAngle(10);
   eitem->setPos(100, 10);
   eitem->setRotation(60);
   eitem->setTransformations({xform});

   auto *litem = scene.addLine(QLineF(0, 0, 100, 100), QPen(Qt::red));
   litem->setPos(10, 10);
   litem->setRotation(100);

   scene.createItemGroup({eitem, litem});

   auto *ritem = scene.addRect(QRect(10, 0, 100, 100), QPen(Qt::blue), QBrush(Qt::red));
   ritem->setPos(10, 100);
   ritem->setRotation(10);

   QPainterPath path;
   path.moveTo(100, 100);
   path.lineTo(10, 0);
   path.addRect(QRect(0, 0, 100, 22));
   auto *pitem = scene.addPath(path, QPen(Qt::green), QBrush(Qt::black));
   pitem->setPos(100, 22);
   pitem->setRotation(120);

   scene.addPixmap(makePixmap());

   auto *stitem = scene.addSimpleText("Simple Text", {"arial", 17});
   stitem->setPos(-50,-10);
   stitem->setPen(QPen(Qt::darkGreen));

   auto *titem = scene.addText("Text", {"arial", 17, true});
   titem->setPos(-100, 0);
   titem->setDefaultTextColor(Qt::darkYellow);

   auto const flags = QGraphicsItem::ItemIsMovable
         | QGraphicsItem::ItemIsSelectable
         | QGraphicsItem::ItemIsFocusable;
   for(auto *it : scene.items())
      it->setFlags(flags);

   QByteArray data;
   QObject::connect(&save, &QPushButton::clicked, [&scene, &data](){
      qDebug()<< "writing ...";
      QBuffer dev(&data);
      if (dev.open(QIODevice::WriteOnly)) {
         QDataStream out(&dev);
         out << scene.items(Qt::AscendingOrder);
         scene.clear();
         qDebug() << "done writing";
      }
   });
   QObject::connect(&load, &QPushButton::clicked, [&scene, &data](){
      qDebug()<< "reading ...";
      QBuffer dev(&data);
      if (dev.open(QIODevice::ReadOnly)) {
         QList<QGraphicsItem *> items;
         QDataStream in(&dev);
         in >> items;
         for (auto *item : items)
            scene.addItem(item);
         qDebug() << "done reading";
      }
   });

   ui.show();
   return a.exec();
}

// Implementation Specializations

static bool specInit = []{
   qRegisterMetaType<QGraphicsRotation>();
   qRegisterMetaType<QGraphicsScale>();
   registerMapping<QGraphicsItemGroup>();
   registerMapping<QGraphicsLineItem>();
   registerMapping<QGraphicsObject>();
   registerMapping<QGraphicsPixmapItem>();
   registerMapping<QGraphicsEllipseItem>();
   registerMapping<QGraphicsPathItem>();
   registerMapping<QGraphicsPolygonItem>();
   registerMapping<QGraphicsRectItem>();
   registerMapping<QGraphicsSimpleTextItem>();
   registerMapping<QGraphicsTextItem>();
   return true;
}();

QDataStream &operator<<(QDataStream &out, const QGraphicsEllipseItem &g) {
   out << static_cast<const QAbstractGraphicsShapeItem &>(g);
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
   in >> static_cast<QAbstractGraphicsShapeItem &>(g);
   ItemStream(in, g) >> &QGI::setText >> &QGI::setFont;
   return in;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsItemGroup &g) {
   return out << static_cast<const QGraphicsItem &>(g);
}

QDataStream &operator>>(QDataStream &in, QGraphicsItemGroup &g) {
   return in >> static_cast<QGraphicsItem &>(g);
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

QDataStream &operator<<(QDataStream &out, const QGraphicsObject &g) {
   static const QByteArrayList excluded{
      "effect", "enabled", "opacity", "parent", "pos", "rotation", "scale",
      "transformOriginPoint", "visible", "x", "y", "z", "children"
   };
   out << static_cast<const QGraphicsItem &>(g);
   saveProperties(out, &g, excluded);
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsObject &g) {
   in >> static_cast<QGraphicsItem &>(g);
   loadProperties(in, &g);
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

QDataStream &operator<<(QDataStream &out, const QGraphicsTextItem &g) {
   out << static_cast<const QGraphicsObject &>(g)
       << g.font() << g.textWidth() << g.defaultTextColor()
       << g.toHtml()
       << g.tabChangesFocus() << g.openExternalLinks() << g.textInteractionFlags();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsTextItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   in >> static_cast<QGraphicsObject &>(g);
   ItemStream(in, g) >> &QGI::setFont >> &QGI::setTextWidth >> &QGI::setDefaultTextColor
                     >> &QGI::setHtml
                     >> &QGI::setTabChangesFocus >> &QGI::setOpenExternalLinks >> &QGI::setTextInteractionFlags;
   return in;
}

// Implementation Core
#include <set>

void saveProperties(QDataStream &ds, const QObject *obj, const QByteArrayList &excluded) {
   QVariantMap map;
   auto const &mo = obj->metaObject();
   for (int i = 0; i < mo->propertyCount(); ++i) {
      auto const &mp = mo->property(i);
      if (!mp.isStored(obj))
         continue;
      if (!excluded.contains(mp.name())) {
         auto prop = mp.read(obj);
         if (!prop.isNull())
            map.insert(QLatin1String(mp.name()), prop);
      }
   }
   for (auto &name : obj->dynamicPropertyNames())
      map.insert(QLatin1String(name), obj->property(name));
   qDebug() << obj->metaObject()->className() << map;
   ds << map;
}

void loadProperties(QDataStream &ds, QObject *obj) {
   QVariantMap map;
   ds >> map;
   for (auto it = map.cbegin(); it != map.cend(); ++it)
      obj->setProperty(it.key().toLatin1(), it.value());
}

QDataStream &operator<<(QDataStream &ds, const QGraphicsTransform *obj) {
   ds << obj->metaObject()->className();
   saveProperties(ds, obj);
   return ds;
}

QDataStream &operator>>(QDataStream &ds, QGraphicsTransform *&obj) {
   QByteArray className;
   ds >> className;
   auto const type = QMetaType::type(className);
   obj = static_cast<QGraphicsTransform*>(QMetaType::create(type));
   if (obj)
      loadProperties(ds, obj);
   return ds;
}

struct pair { int typeId; int itemType; };
struct CoreData {
   QReadWriteLock mappingLock;
   QVector<pair> mapping;
};
Q_GLOBAL_STATIC(CoreData, coreData)

int getTypeIdForItemType(int itemType) {
   if (auto *const d = coreData()) {
      QReadLocker lock(&d->mappingLock);
      for (auto &m : qAsConst(d->mapping))
         if (m.itemType == itemType)
            return m.typeId;
   }
   return QMetaType::UnknownType;
}

void registerMapping(int typeId, int itemType) {
   if (getTypeIdForItemType(itemType) == typeId)
      return;
   if (auto *const d = coreData()) {
      QWriteLocker lock(&d->mappingLock);
      for (auto &m : qAsConst(d->mapping))
         if (m.typeId == typeId && m.itemType == itemType)
            return;
      d->mapping.push_back({typeId, itemType});
   }
}

QDataStream &operator<<(QDataStream &ds, const QList<QGraphicsItem*> &list) {
   std::set<QGraphicsItem*> seen;
   QList<QGraphicsItem*> items;
   struct State { QList<QGraphicsItem*>::const_iterator it, end; };
   QVector<State> stack;
   stack.push_back({list.begin(), list.end()});
   while (!stack.isEmpty()) {
      auto &level = stack.back();
      while (level.it != level.end) {
         QGraphicsItem *item = *level.it++;
         if (!item || seen.find(item) != seen.end())
            continue; // skip empty items and seen items
         if (stack.size() == 1) // push direct items only
            items.push_back(item);
         seen.insert(item);
         const auto &children = item->childItems();
         if (!children.isEmpty()) {
            stack.push_back({children.begin(), children.end()});
            break;
         }
      }
      if (level.it == level.end)
         stack.pop_back();
   }
   ds << quint32(items.size());
   for (auto *item : items)
      ds << item;
   return ds;
}

QDataStream &operator<<(QDataStream &ds, const QGraphicsItem *item) {
   int const typeId = getTypeIdForItemType(item->type());
   if (typeId != QMetaType::UnknownType)
      QMetaType::save(ds, typeId, item);
   else
      ds << int(QMetaType::UnknownType);
   return ds;
}

QDataStream &operator>>(QDataStream &ds, QGraphicsItem *&item) { 
   int itemType = 0;
   auto read = ds.device()->peek(reinterpret_cast<char*>(&itemType), sizeof(itemType));
   if (read != sizeof(itemType)) {
      ds.setStatus(QDataStream::ReadPastEnd);
      return ds;
   }
   itemType = qFromBigEndian(itemType);
   int const typeId = getTypeIdForItemType(itemType);
   item = static_cast<QGraphicsItem*>(QMetaType::create(typeId));
   if (item)
      QMetaType::load(ds, typeId, item);
   return ds;
}

QDataStream &operator<<(QDataStream &out, const QGraphicsItem &g) {
   out << int(g.type())
       << ItemStream::CurVersion
       << g.pos()
       << g.scale()
       << g.rotation()
       << g.transform()
       << g.transformations()
       << g.transformOriginPoint()
       << g.flags()
       << g.isEnabled()
       << g.isSelected()
       << g.isVisible()
       << g.acceptDrops()
       << g.acceptHoverEvents()
       << g.acceptTouchEvents()
       << g.acceptedMouseButtons()
       << g.filtersChildEvents()
       << g.cursor()
       << g.inputMethodHints()
       << g.opacity()
       << g.boundingRegionGranularity()
       << g.toolTip()
       << g.zValue()
       << g.childItems();
   return out;
}

QDataStream &operator>>(QDataStream &in, QGraphicsItem &g) {
   using QGI = std::decay<decltype(g)>::type;
   int type;
   ItemStream::Version version;
   QTransform transform;
   QList<QGraphicsItem*> children;

   in    >> type
         >> version;
   Q_ASSERT(g.type() == type);
   if (version > ItemStream::CurVersion) {
      qWarning() << "unsupported QGraphicsItem version" << version << "maximum is:" << ItemStream::CurVersion;
      in.setStatus(QDataStream::ReadCorruptData);
      return in;
   }
   ItemStream::setVersion(g, version);
   ItemStream iin(in, g);
   iin   >> &QGI::setPos
         >> &QGI::setScale
         >> &QGI::setRotation;
   in    >> transform;
   g.setTransform(transform);
   iin   >> &QGI::setTransformations
         >> &QGI::setTransformOriginPoint
         >> &QGI::setFlags
         >> &QGI::setEnabled
         >> &QGI::setSelected;
   if (version >= 1) {
      iin >> &QGI::setVisible
            >> &QGI::setAcceptDrops
            >> &QGI::setAcceptHoverEvents
            >> &QGI::setAcceptTouchEvents
            >> &QGI::setAcceptedMouseButtons
            >> &QGI::setFiltersChildEvents
            >> &QGI::setCursor
            >> &QGI::setInputMethodHints
            >> &QGI::setOpacity
            >> &QGI::setBoundingRegionGranularity
            >> &QGI::setToolTip;
   }
   iin   >> &QGI::setZValue;
   in    >> children;

   for (auto *c : qAsConst(children))
      c->setParentItem(&g);
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
