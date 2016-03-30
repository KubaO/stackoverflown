// https://github.com/KubaO/stackoverflown/tree/master/questions/host-lookup-36319049
#include <QtWidgets>
#include <QtNetwork>

class IfLookup : public QObject {
   Q_OBJECT
   int m_id;
   QTimer m_timer;
   void abort() {
      if (m_timer.isActive())
         QHostInfo::abortHostLookup(m_id);
      m_timer.stop();
   }
   Q_SLOT void lookupResult(const QHostInfo & host) {
      m_timer.stop();
      if (host.error() != QHostInfo::NoError)
         return hasResult(Error);
      for (auto ifAddr : QNetworkInterface::allAddresses())
         if (host.addresses().contains(ifAddr))
            return hasResult(Local);
      return hasResult(NonLocal);
   }
public:
   enum Result { Local, NonLocal, TimedOut, Error };
   IfLookup(QObject * parent = 0) : QObject(parent) {
      connect(&m_timer, &QTimer::timeout, this, [this]{
         abort();
         emit hasResult(TimedOut);
      });
   }
   Q_SIGNAL void hasResult(Result);
   Q_SLOT void lookup(QString name) {
      abort();
      name = name.trimmed().toUpper();
      QHostAddress addr(name);
      if (!addr.isNull()) {
         if (addr.isLoopback() || QNetworkInterface::allAddresses().contains(addr))
            return hasResult(Local);
         return hasResult(NonLocal);
      }
      if (QHostInfo::localHostName() == name)
         return hasResult(Local);
      m_id = QHostInfo::lookupHost(name, this, SLOT(lookupResult(QHostInfo)));
      m_timer.start(500);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QFormLayout layout{&w};
   QLineEdit address;
   layout.addRow("Address to look up", &address);
   QLabel result;
   layout.addRow("Result", &result);
   QPushButton lookup{"Lookup"};
   layout.addRow(&lookup);
   lookup.setDefault(true);
   w.show();

   IfLookup ifLookup;
   QObject::connect(&lookup, &QPushButton::clicked, [&]{
      result.clear();
      ifLookup.lookup(address.text());
   });
   QObject::connect(&ifLookup, &IfLookup::hasResult, [&](IfLookup::Result r){
      static const QMap<IfLookup::Result, QString> msgs = {
         { IfLookup::Local, "Local" }, { IfLookup::NonLocal, "Non-Local" },
         { IfLookup::TimedOut, "Timed Out" }, { IfLookup::Error, "Lookup Error" }
      };
      result.setText(msgs.value(r));
   });

   return app.exec();
}

#include "main.moc"
