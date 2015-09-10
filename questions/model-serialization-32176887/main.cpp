// https://github.com/KubaO/stackoverflown/tree/master/questions/model-serialization-32176887
#include <QtGui>

struct BasicTraits  {
    BasicTraits() {}
    /// The base model that the serializer operates on
    typedef QAbstractItemModel Model;
    /// The streamable representation of model's configuration
    typedef bool ModelConfig;
    /// The streamable representation of an item's data
    typedef QMap<int, QVariant> Roles;
    /// The streamable representation of a section of model's header data
    typedef Roles HeaderRoles;
    /// Returns a streamable representation of an item's data.
    Roles itemData(const Model * model, const QModelIndex & index) {
        return model->itemData(index);
    }
    /// Sets the item's data from the streamable representation.
    bool setItemData(Model * model, const QModelIndex & index, const Roles & data) {
        return model->setItemData(index, data);
    }
    /// Returns a streamable representation of a model's header data.
    HeaderRoles headerData(const Model * model, int section, Qt::Orientation ori) {
        Roles data;
        data.insert(Qt::DisplayRole, model->headerData(section, ori));
        return data;
    }
    /// Sets the model's header data from the streamable representation.
    bool setHeaderData(Model * model, int section, Qt::Orientation ori, const HeaderRoles & data) {
        return model->setHeaderData(section, ori, data.value(Qt::DisplayRole));
    }
    /// Should horizontal header data be serialized?
    bool doHorizontalHeaderData() const { return true; }
    /// Should vertical header data be serialized?
    bool doVerticalHeaderData() const { return false; }
    /// Sets the number of rows and columns for children on a given parent item.
    bool setRowsColumns(Model * model, const QModelIndex & parent, int rows, int columns) {
        bool rc = model->insertRows(0, rows, parent);
        if (columns > 1) rc = rc && model->insertColumns(1, columns-1, parent);
        return rc;
    }
    /// Returns a streamable representation of the model's configuration.
    ModelConfig modelConfig(const Model *) {
        return true;
    }
    /// Sets the model's configuration from the streamable representation.
    bool setModelConfig(Model *, const ModelConfig &) {
        return true;
    }
};

struct Status {
    enum SubStatus { StreamOk = 1, ModelOk = 2, IgnoreModelFailures = 4 };
    QFlags<SubStatus> flags;
    Status(SubStatus s) : flags(StreamOk | ModelOk | s) {}
    Status() : flags(StreamOk | ModelOk) {}
    bool ok() const {
        return (flags & StreamOk && (flags & IgnoreModelFailures || flags & ModelOk));
    }
    bool operator()(QDataStream & str) {
        return stream(str.status() == QDataStream::Ok);
    }
    bool operator()(Status s) {
        if (flags & StreamOk && ! (s.flags & StreamOk)) flags ^= StreamOk;
        if (flags & ModelOk && ! (s.flags & ModelOk)) flags ^= ModelOk;
        return ok();
    }
    bool model(bool s) {
        if (flags & ModelOk && !s) flags ^= ModelOk;
        return ok();
    }
    bool stream(bool s) {
        if (flags & StreamOk && !s) flags ^= StreamOk;
        return ok();
    }
};

template <class Tr = BasicTraits> class ModelSerializer {
    enum ItemType { HasData = 1, HasChildren = 2 };
    Q_DECLARE_FLAGS(ItemTypes, ItemType)
    Tr m_traits;
    Status saveHeaders(QDataStream & s, const typename Tr::Model * model, int count, Qt::Orientation ori) {
        Status st;
        if (!st(s << (qint32)count)) return st;
        for (int i = 0; i < count; ++i)
            if (!st(s << m_traits.headerData(model, i, ori))) return st;
        return st;
    }
    Status loadHeaders(QDataStream & s, typename Tr::Model * model, Qt::Orientation ori, Status st) {
        qint32 count;
        if (!st(s >> count)) return st;
        for (qint32 i = 0; i < count; ++i) {
            typename Tr::HeaderRoles data;
            if (!st(s >> data)) return st;
            if (!st.model(m_traits.setHeaderData(model, i, ori, data))) return st;
        }
        return st;
    }
    Status saveData(QDataStream & s, const typename Tr::Model * model, const QModelIndex & parent) {
        Status st;
        ItemTypes types;
        if (parent.isValid()) types |= HasData;
        if (model->hasChildren(parent)) types |= HasChildren;
        if (!st(s << (quint8)types)) return st;
        if (types & HasData) s << m_traits.itemData(model, parent);
        if (! (types & HasChildren)) return st;
        auto rows = model->rowCount(parent);
        auto columns = model->columnCount(parent);
        if (!st(s << (qint32)rows << (qint32)columns)) return st;
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < columns; ++j)
                if (!st(saveData(s, model, model->index(i, j, parent)))) return st;
        return st;
    }
    Status loadData(QDataStream & s, typename Tr::Model * model, const QModelIndex & parent, Status st) {
        quint8 rawTypes;
        if (!st(s >> rawTypes)) return st;
        ItemTypes types { rawTypes };
        if (types & HasData) {
            typename Tr::Roles data;
            if (!st(s >> data)) return st;
            if (!st.model(m_traits.setItemData(model, parent, data))) return st;
        }
        if (! (types & HasChildren)) return st;
        qint32 rows, columns;
        if (!st(s >> rows >> columns)) return st;
        if (!st.model(m_traits.setRowsColumns(model, parent, rows, columns))) return st;
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < columns; ++j)
                if (!st(loadData(s, model, model->index(i, j, parent), st))) return st;
        return st;
    }
public:
    ModelSerializer() {}
    ModelSerializer(const Tr & traits) : m_traits(traits) {}
    ModelSerializer(Tr && traits) : m_traits(std::move(traits)) {}
    ModelSerializer(const ModelSerializer &) = default;
    ModelSerializer(ModelSerializer &&) = default;
    Status save(QDataStream & stream, const typename Tr::Model * model) {
        Status st;
        auto version = stream.version();
        stream.setVersion(QDataStream::Qt_5_4);
        if (!st(stream << (quint8)0)) return st; // format
        if (!st(stream << m_traits.modelConfig(model))) return st;
        if (!st(saveData(stream, model, QModelIndex()))) return st;
        auto hor = m_traits.doHorizontalHeaderData();
        if (!st(stream << hor)) return st;
        if (hor && !st(saveHeaders(stream, model, model->rowCount(), Qt::Horizontal))) return st;
        auto ver = m_traits.doVerticalHeaderData();
        if (!st(stream << ver)) return st;
        if (ver && !st(saveHeaders(stream, model, model->columnCount(), Qt::Vertical))) return st;
        stream.setVersion(version);
        return st;
    }
    Status load(QDataStream & stream, typename Tr::Model * model, Status st = Status()) {
        auto version = stream.version();
        stream.setVersion(QDataStream::Qt_5_4);
        quint8 format;
        if (!st(stream >> format)) return st;
        if (!st.stream(format == 0)) return st;
        typename Tr::ModelConfig config;
        if (!st(stream >> config)) return st;
        if (!st.model(m_traits.setModelConfig(model, config))) return st;
        if (!st(loadData(stream, model, QModelIndex(), st))) return st;
        bool hor;
        if (!st(stream >> hor)) return st;
        if (hor && !st(loadHeaders(stream, model, Qt::Horizontal, st))) return st;
        bool ver;
        if (!st(stream >> ver)) return st;
        if (ver && !st(loadHeaders(stream, model, Qt::Vertical, st))) return st;
        stream.setVersion(version);
        return st;
    }
};

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    QStringList srcData;
    for (int i = 0; i < 1000; ++i) srcData << QString::number(i);
    QStringListModel src {srcData}, dst;
    ModelSerializer<> ser;
    QByteArray buffer;
    QDataStream sout(&buffer, QIODevice::WriteOnly);
    ser.save(sout, &src);
    QDataStream sin(buffer);
    ser.load(sin, &dst);
    Q_ASSERT(srcData == dst.stringList());
}

