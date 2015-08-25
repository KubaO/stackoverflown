#include <QApplication>
#include <QStandardItemModel>
#include <QTreeView>

//  |                         |
//  +--<Class>                +-- PrototypeItem
//  |     +-- Type  <Type>    |     +-- QStandardItem TypeItem
//  |     +-- Prop1 <Value1>  |     +-- QStandardItem QStandardItem
//  |     .     .      .      |     .        .               .
//  .                               .

static const auto TypeKey = QStringLiteral("Type");

/// A factory that keeps item prototypes and makes their clones
class ItemFactory {
    Q_DISABLE_COPY(ItemFactory)
public:
    typedef QSharedPointer<QStandardItem> ItemPtr;
    typedef QPair<QString, QString> ClassAndType;
    QMap<ClassAndType, ItemPtr> m_map;
public:
    ItemFactory() {}
    void add(QStandardItem * item) {
        auto class_ = item->text();
        auto type_ = item->child(0, 1)->text();
        m_map.insert(ClassAndType(class_, type_), ItemPtr(item));
    }
    bool has(const QString & class_, const QString & type_) const {
        return m_map.find(ClassAndType(class_, type_)) != m_map.end();
    }
    QStandardItem * make(const QString & class_, const QString & type_, QStandardItem * item = nullptr) const;
    QStringList keys(const QString & class_) const {
        QStringList result;
        for (auto const & key : m_map.keys())
            if (key.first == class_) result << key.second;
        return result;
    }
};

Q_GLOBAL_STATIC(ItemFactory, itemFactory)

/// An item that represents the type of the organism. It informs the
/// parent of type changes, and provides the list of available types
/// for a given organism class form the factory.
class TypeItem : public QStandardItem {
    QString getClass() const { return parent()->text(); }
public:
    TypeItem(const QString & type_) : QStandardItem(type_) {}
    TypeItem * clone() const { return new TypeItem(text()); }
    QVariant data(int role) const Q_DECL_OVERRIDE {
        return (role == Qt::UserRole + 1) ? itemFactory->keys(getClass()) : QStandardItem::data(role);
    }
    void setData(const QVariant & value, int role) Q_DECL_OVERRIDE;
};

/// An item that can clone its children and replace itself with other
/// prototypes based on the type.
class PrototypeItem : public QStandardItem {
    friend class TypeItem;
    QString getType() const { return child(0, 1)->text(); }
    QMap<QString, QVariant> getKeyValues() const {
        QMap<QString, QVariant> result;
        for (int i = 1; i < rowCount(); ++i)
            result.insert(child(i, 0)->text(), child(i,1)->data(Qt::DisplayRole));
        return result;
    }
    /// Replace the prototype with a new one, preserving property values.
    void newType(const QString & type) {
        if (! itemFactory->has(text(), type)) return;
        auto keyValues = getKeyValues();
        removeRows(0, rowCount());
        itemFactory->make(text(), type, this);
        Q_ASSERT(child(0, 1)->text() == type);
        for (int r = rowCount() - 1; r >= 1; --r) {
            auto it = keyValues.find(child(r, 0)->text());
            if (it != keyValues.end())
                child(r, 1)->setData(it.value(), Qt::EditRole);
        }
    }
    PrototypeItem() {}
public:
    virtual QStandardItem * cloneInto(QStandardItem * item) const {
        for (int r = rowCount() - 1; r >= 0; --r)
            for (int c = columnCount() - 1; c >= 0; --c)
                item->setChild(r, c, child(r, c)->clone());
        return item;
    }
    QStandardItem * clone() const Q_DECL_OVERRIDE {
        auto item = new PrototypeItem;
        *item = *this;
        return cloneInto(item); // Clone into a copy of ourselves
    }
    PrototypeItem(const QString & class_, const QString & type_,
                  const QVariantList & keyValues = QVariantList()) :
        QStandardItem(class_) {
        setChild(0, 0, new QStandardItem(TypeKey));
        setChild(0, 1, new TypeItem(type_));
        for (int i = (keyValues.size()/2)-1; i >= 0; --i) {
            setChild(1 + i, 0, new QStandardItem(keyValues.at(i*2).toString()));
            auto dataItem = new QStandardItem;
            dataItem->setData(keyValues.at(i*2 + 1), Qt::EditRole);
            setChild(1 + i, 1, dataItem);
        }
    }
};

void TypeItem::setData(const QVariant & value, int role) {
    const auto oldType = text();
    if (role != Qt::UserRole + 1) QStandardItem::setData(value, role);
    auto prototype = dynamic_cast<PrototypeItem*>(parent());
    if (text() != oldType) prototype->newType(text());
}

QStandardItem * ItemFactory::make(const QString & class_, const QString & type_, QStandardItem * item) const {
    auto it = m_map.find(ClassAndType(class_, type_));
    if (it == m_map.end()) return nullptr;
    if (!item) return (*it)->clone();
    return dynamic_cast<PrototypeItem*>((*it).data())->cloneInto(item);
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    itemFactory->add(new PrototypeItem("Plant", "venus", QVariantList() <<
                                       "avgFlyI" << 0.0 << "avgHeight" << 0.0));
    itemFactory->add(new PrototypeItem("Plant", "daffodil", QVariantList() <<
                                       "avgHeight" << 0.0));
    QTreeView view;
    QStandardItemModel model;
    model.setColumnCount(2);
    model.appendRow(itemFactory->make("Plant", "venus"));
    view.setModel(&model);
    view.show();
    return a.exec();
}
