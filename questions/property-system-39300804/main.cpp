// https://github.com/KubaO/stackoverflown/tree/master/questions/property-system-39300804
#if 0

#include <map>
#include <string>
#include <cassert>
#include <boost/variant.hpp>

class WithProperties {
public:
    using variant = boost::variant<std::string, int, double, bool>;
    template <typename T> T property(const char * name) const {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) return boost::get<T>(it->second);
        return T{};
    }
    void setProperty(const char * name, const variant & value) {
        m_properties[name] = value;
    }
    std::vector<std::string> propertyNames() const {
        std::vector<std::string> keys;
        keys.reserve(m_properties.size());
        for (auto prop : m_properties)
            keys.push_back(prop.first);
        return keys;
    }
private:
    std::map<std::string, variant> m_properties;
};

int main() {
    WithProperties prop;
    prop.setProperty("down", true);
    prop.setProperty("angle", 35.0);
    prop.setProperty("name", std::string{"foo"});
    assert(prop.property<bool>("down") == true);
    assert(prop.property<double>("angle") == 35.0);
    assert(prop.property<std::string>("name") == "foo");
}

#endif

#if 0

#include <QtCore>

class WithProperties {
public:
    QVariant property(const char * name) const {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) return *it;
        return QVariant{};
    }
    void setProperty(const char * name, const QVariant & value) {
        m_properties[name] = value;
    }
    QList<QByteArray> propertyNames() const {
        return m_properties.keys();
    }
private:
    QMap<QByteArray, QVariant> m_properties;
};

int main() {
    WithProperties prop;
    prop.setProperty("down", true);
    prop.setProperty("angle", 35.0);
    prop.setProperty("name", "foo");
    Q_ASSERT(prop.property("down") == true);
    Q_ASSERT(prop.property("angle") == 35.0);
    Q_ASSERT(prop.property("name") == "foo");
}

#endif

#if 1

#include <QtCore>
#include <cstring>

QMetaProperty findMetaProperty(const QMetaObject * obj, const char * name) {
    auto count = obj->propertyCount();
    for (int i = 0; i < count; ++i) {
        auto prop = obj->property(i);
        if (strcmp(prop.name(), name) == 0)
            return prop;
    }
    return QMetaProperty{};
}

class WithProperties {
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName)
    QString m_name;
public:
    QString name() const { return m_name; }
    void setName(const QString & name) { m_name = name; }
    QVariant property(const char * name) const {
        auto metaProperty = findMetaProperty(&staticMetaObject, name);
        if (metaProperty.isValid())
            return metaProperty.readOnGadget(this);
        auto it = m_properties.find(name);
        if (it != m_properties.end()) return *it;
        return QVariant{};
    }
    void setProperty(const char * name, const QVariant & value) {
        auto metaProperty = findMetaProperty(&staticMetaObject, name);
        if (metaProperty.isValid())
            return (void)metaProperty.writeOnGadget(this, value);
        m_properties[name] = value;
    }
    QList<QByteArray> dynamicPropertyNames() const {
        return m_properties.keys();
    }
private:
    QMap<QByteArray, QVariant> m_properties;
};

int main() {
    WithProperties prop;
    prop.setProperty("down", true);
    prop.setProperty("angle", 35.0);
    prop.setProperty("name", "foo");
    Q_ASSERT(prop.property("down") == true);
    Q_ASSERT(prop.property("angle") == 35.0);
    Q_ASSERT(prop.property("name") == "foo");
    Q_ASSERT(prop.dynamicPropertyNames().size() == 2);
}
#include "main.moc"

#endif

