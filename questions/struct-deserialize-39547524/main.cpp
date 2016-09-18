// https://github.com/KubaO/stackoverflown/tree/master/questions/struct-deserialize-39547524
#include <QtCore>
#include <string>

struct Example {
    int field1;
    double field2;
    bool field3;
    std::string field4;
    Q_GADGET
    Q_PROPERTY(int field1 MEMBER field1)
    Q_PROPERTY(double field2 MEMBER field2)
    Q_PROPERTY(bool field3 MEMBER field3)
    Q_PROPERTY(QString field4 READ getField4 WRITE setField4)
public:
    void setField4(const QString & val) { field4 = val.toStdString(); }
    QString getField4() const { return QString::fromStdString(field4); }
};

template <typename T>
void set(T & gadget, const QStringList & data)
{
    for (auto & entry : data) set(gadget, entry);
}

template <typename T>
void set(T & gadget, const QString & data)
{
    const QMetaObject & mo = gadget.staticMetaObject;
    auto const fields = data.split('=');
    if (fields.length() != 2) return;
    auto field = fields[0].trimmed();
    auto value = fields[1].trimmed();
    auto i = mo.indexOfProperty(field.toUtf8());
    if (i < 0) return;
    auto prop = mo.property(i);
    prop.writeOnGadget(&gadget, value);
}

int main() {
    Example ex;
    set(ex, {"field1 = 3", "field2 = 1.5", "field3 = true", "field4 = foo bar"});
    Q_ASSERT(ex.field1 == 3);
    Q_ASSERT(ex.field2 == 1.5);
    Q_ASSERT(ex.field3 == true);
    Q_ASSERT(ex.field4 == "foo bar");
}
#include "main.moc"
