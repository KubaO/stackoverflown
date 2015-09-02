#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QSettings>
#include <QVariant>

namespace VkService {

class MusicOwnerA
{
    friend QDataStream &operator <<(QDataStream &stream, const MusicOwnerA &val);
    friend QDataStream &operator >>(QDataStream &stream, MusicOwnerA &val);
    friend QDebug operator<< (QDebug d, const MusicOwnerA &owner);
public:
    MusicOwnerA() : id(0) {}
    int id;
    QString name;
};

QDataStream &operator <<(QDataStream &stream, const VkService::MusicOwnerA &val)
{
    stream << val.id;
    stream << val.name;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, VkService::MusicOwnerA &val)
{
    stream >> val.id;
    stream >> val.name;
    return stream;
}

QDebug operator <<(QDebug d, const VkService::MusicOwnerA &owner)
{
    d << "VkService::MusicOwnerA("
      << owner.id << ","
      << owner.name << ")";
    return d;
}

}

Q_DECLARE_METATYPE(VkService::MusicOwnerA)

void Save() {
    QSettings s;
    s.beginWriteArray("bookmarks");
    int index = 0;
    for (int i = 0; i < 100; ++i){
        if (random() % 5 == 0) {
            s.setArrayIndex(index);
            VkService::MusicOwnerA owner;
            owner.id = i;
            owner.name ="Hello world";
            s.setValue("owner", QVariant::fromValue(owner));
            qDebug() << "Saved" << i << ":" << owner;
            ++index;
        }
    }
    s.endArray();
    qDebug() << "Saved" << index << "elements";
}

void Load() {
    QSettings s;
    int max = s.beginReadArray("bookmarks");
    qDebug() << "To load" << max << "elements";
    for (int i = 0; i < max; ++i){
        s.setArrayIndex(i);
        VkService::MusicOwnerA owner = s.value("owner").value<VkService::MusicOwnerA>();
        qDebug() << "\tLoaded" << i << ":" << owner;
    }
    s.endArray();
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    a.setOrganizationDomain("16549302.stackoverflow.com");
    qRegisterMetaTypeStreamOperators<VkService::MusicOwnerA>("VkService::MusicOwnerA");
    Load();
    Save();
}
