// https://github.com/KubaO/stackoverflown/tree/master/questions/stream-qwidgetlistitem-51403419
#include <QtWidgets>

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

//

class Layer : public QListWidgetItem {
   Q_GADGET
public:
   enum LayerType { // names must match type names
      RasterLayer = QListWidgetItem::UserType,
      VectorLayer
   };
   Q_ENUM(LayerType)

   virtual QGraphicsItem *it() = 0;
   const QGraphicsItem *it() const { return const_cast<Layer*>(this)->it(); }
   int typeId() const { return enumToTypeId<LayerType>(type()); }
   void write(QDataStream&) const override;
   void read(QDataStream&) override;
   QListWidgetItem *clone() const override final;

   void setZValue(int z) { it()->setZValue(z); }
   void setParentItem(Layer *parent) { it()->setParentItem(parent->it()); }
   void setParentItem(QGraphicsItem *parent) { it()->setParentItem(parent); }
   void setSelected(bool sel) { it()->setSelected(sel); }
   void setPos(const QPointF &pos) { it()->setPos(pos); }

   Layer(const Layer &);
   QString name() const { return m_name; }
   void setName(const QString &n) { m_name = n; }
protected:
   using Format = quint8;
   Layer(const QString &name, int type);
   static void invalidFormat(QDataStream &);

private:
   QString m_name;
};

//

Layer::Layer(const Layer &o) : Layer(o.name(), o.type()) {}

Layer::Layer(const QString &name, int type) :
   QListWidgetItem(nullptr, type),
   m_name(name)
{}

QListWidgetItem *Layer::clone() const {
   const QMetaType mt(typeId());
   Q_ASSERT(mt.isValid());
   return reinterpret_cast<QListWidgetItem*>(mt.create(this));
}

void Layer::write(QDataStream &ds) const {
   ds << (qint32)type() << (Format)0 << m_name << it()->pos();
   QListWidgetItem::write(ds);
}

void Layer::read(QDataStream &ds) {
   qint32 type_;
   Format format_;
   QPointF pos_;
   ds >> type_ >> format_;
   Q_ASSERT(type_ == type());
   if (format_ >= 0) {
      ds >> m_name >> pos_;
      setPos(pos_);
      QListWidgetItem::read(ds);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

void Layer::invalidFormat(QDataStream &ds) {
   ds.setStatus(QDataStream::ReadCorruptData);
}

QDataStream &operator<<(QDataStream &ds, const Layer *l) {
   return ds << *l;
}

QDataStream &operator>>(QDataStream &ds, Layer *&l) {
   qint32 type = 0;
   auto read = ds.device()->peek(reinterpret_cast<char*>(&type), sizeof(type));
   if (read != sizeof(type)) {
      ds.setStatus(QDataStream::ReadPastEnd);
      return ds;
   }
   int typeId = enumToTypeId<Layer::LayerType>(qFromBigEndian(type));
   QMetaType mt(typeId);
   l = mt.isValid() ? reinterpret_cast<Layer*>(mt.create()) : nullptr;
   if (l)
      ds >> *l;
   else
      ds.setStatus(QDataStream::ReadCorruptData);
   return ds;
}

//

class RasterLayer : public Layer, public QGraphicsPixmapItem {
public:
   QGraphicsItem *it() override { return this; }
   void write(QDataStream &) const override;
   void read(QDataStream &) override;
   RasterLayer(const RasterLayer &);
   RasterLayer(const QString &name = {});
};
Q_DECLARE_METATYPE(RasterLayer)

// implementation

static int rasterOps = qRegisterMetaTypeStreamOperators<RasterLayer>();

RasterLayer::RasterLayer(const RasterLayer &o) :
   Layer(o),
   QGraphicsPixmapItem(o.pixmap())
{}

RasterLayer::RasterLayer(const QString &name) : Layer(name, Layer::RasterLayer) {}

void RasterLayer::write(QDataStream &ds) const {
   Layer::write(ds);
   ds << Format(0) << pixmap();
}

void RasterLayer::read(QDataStream &ds) {
   Layer::read(ds);
   Format format_;
   QPixmap pix_;
   ds >> format_;
   if (format_ >= 0) {
      ds >> pix_;
      setPixmap(pix_);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

//

class VectorLayer : public Layer, public QGraphicsPathItem {
public:
   QGraphicsItem *it() override { return this; }
   void write(QDataStream &) const override;
   void read(QDataStream &) override;
   VectorLayer(const VectorLayer &);
   VectorLayer(const QString &name = {});
};
Q_DECLARE_METATYPE(VectorLayer)

// implementation

static int vectorOps = qRegisterMetaTypeStreamOperators<VectorLayer>();

VectorLayer::VectorLayer(const VectorLayer &o) :
   Layer(o),
   QGraphicsPathItem(o.path())
{}

VectorLayer::VectorLayer(const QString &name) : Layer(name, Layer::VectorLayer) {}

void VectorLayer::write(QDataStream &ds) const {
   Layer::write(ds);
   ds << Format(0) << path();
}

void VectorLayer::read(QDataStream &ds) {
   Layer::read(ds);
   Format format_;
   QPainterPath path_;
   ds >> format_;
   if (format_ >= 0) {
      ds >> path_;
      setPath(path_);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

//

void test0() {
   Q_ASSERT(checkTypeEnum<Layer::LayerType>());
}

void test1(QDataStream &ds) {
   ds.device()->reset();
   RasterLayer raster;
   VectorLayer vector;
   ds << raster << vector;

   ds.device()->reset();
   QVector<Layer*> layers(2);
   Q_ASSERT(!layers[0] && !layers[1]);
   ds >> layers[0] >> layers[1];
   Q_ASSERT(layers[0] && layers[1]);
   qDeleteAll(layers);
}

void test2(QDataStream &ds) {
   ds.device()->reset();
   RasterLayer raster;
   VectorLayer vector;
   QList<Layer*> layers;
   layers << &raster << &vector;
   ds << layers;

   ds.device()->reset();
   layers.clear();
   Q_ASSERT(layers.isEmpty());
   ds >> layers;
   Q_ASSERT(layers.size() == 2);
   Q_ASSERT(!layers.contains({}));
   qDeleteAll(layers);
}

void test3(QDataStream &ds) {
   ds.device()->reset();
   RasterLayer raster;
   VectorLayer vector;
   auto vr = QVariant::fromValue(raster);
   auto vv = QVariant::fromValue(vector);
   ds << vr << vv;

   ds.device()->reset();
   vv.clear();
   vr.clear();
   Q_ASSERT(vr.isNull() && vv.isNull());
   ds >> vr >> vv;
   Q_ASSERT(!vr.isNull() && !vv.isNull());
   Q_ASSERT(vr.userType() == qMetaTypeId<RasterLayer>());
   Q_ASSERT(vv.userType() == qMetaTypeId<VectorLayer>());
}

void test() {
   test0();
   QBuffer buf;
   buf.open(QIODevice::ReadWrite);
   QDataStream ds(&buf);
   test1(ds);
   test2(ds);
   test3(ds);
}

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   test();
}
#include "main.moc"
