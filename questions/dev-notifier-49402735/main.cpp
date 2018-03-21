// https://github.com/KubaO/stackoverflown/tree/master/questions/dev-notifier-49402735
#include <QtCore>
#include <fcntl.h>
#include <boost/optional.hpp>

class DeviceFile : public QFile {
   Q_OBJECT
   boost::optional<QSocketNotifier> m_notifier;
public:
   DeviceFile() {}
   DeviceFile(const QString &name) : QFile(name) {}
   DeviceFile(QObject * parent = {}) : QFile(parent) {}
   DeviceFile(const QString &name, QObject *parent) : QFile(name, parent) {}
   bool open(OpenMode flags) override {
      return
            QFile::isOpen()
            || QFile::open(flags)
            && fcntl(handle(), F_SETFL, O_NONBLOCK) != -1
            && (m_notifier.emplace(this->handle(), QSocketNotifier::Read, this), true)
            && m_notifier->isEnabled()
            && connect(&*m_notifier, &QSocketNotifier::activated, this, &QIODevice::readyRead)
            || (close(), false);
   }
   void close() override {
      m_notifier.reset();
      QFile::close();
   }
};

int main(int argc, char **argv) {
   QCoreApplication app{argc, argv};
   DeviceFile dev("/dev/stdin");
   QObject::connect(&dev, &QIODevice::readyRead, [&]{
      qDebug() << "*";
      if (dev.readAll().contains('q'))
         app.quit();
   });
   if (dev.open(QIODevice::ReadOnly))
      return app.exec();
}

#include "main.moc"
