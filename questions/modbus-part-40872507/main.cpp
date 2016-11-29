// https://github.com/KubaO/stackoverflown/tree/master/questions/modbus-part-40872507
#include <QtWidgets>
#include <QtSerialBus>

namespace Ui {
   struct DeviceDriverViewGUI {
      QPushButton *pbTry = new QPushButton;
      void setupUi(QWidget*) {}
   };
}

struct Equip {};

struct Cfg {
   struct Modbus {
      int porta = 0;
      QString ip = QStringLiteral("127.0.0.1");
      int timeout = 1000;
      int retries = 2;
   } modbus;
};

class MD4040drive : public QObject
{
   Q_OBJECT
public:
   explicit MD4040drive(QObject *parent = nullptr);
   void requestMD4040drive(Equip *equip, const QDateTime &date);
      Q_SIGNAL void end(bool boRun);
private:
   Equip *equip = nullptr;
   QDateTime date;
   QModbusTcpClient modbusDevice{this};
   Cfg cfg;

   Q_SLOT void onStateChanged();
   Q_SLOT void m_conn();
   void sm_conn();
   void getDados_NO_TH() {}
};

MD4040drive::MD4040drive(QObject *parent): QObject(parent)
{
   connect(&modbusDevice, &QModbusClient::stateChanged,this, &MD4040drive::onStateChanged);
}

void MD4040drive::requestMD4040drive(Equip *equip, const QDateTime &date)
{
   this->equip = equip;
   this->date = date;
   sm_conn();
}

void MD4040drive::sm_conn()
{

   if (modbusDevice.state() != QModbusDevice::ConnectedState) {
      modbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, cfg.modbus.porta );
      modbusDevice.setConnectionParameter(QModbusDevice::NetworkAddressParameter, cfg.modbus.ip);
      modbusDevice.setTimeout( this->cfg.modbus.timeout );
      modbusDevice.setNumberOfRetries(this->cfg.modbus.retries);
      qDebug() << "MD4040drive::sm_conn() - try connect";
      if (!modbusDevice.connectDevice()) {
         qDebug()  << "Erro: " << modbusDevice.errorString();
      } else {
         qDebug()  << "Aguardando conexão...";
      }
   }
   else{
      //already connected...
      getDados_NO_TH();
   }
}

class SafeThread : public QThread {
   Q_OBJECT
   using QThread::run;
public:
   using QThread::QThread;
   ~SafeThread() { quit(); wait(); }
};

class DeviceDriverViewGUI : public QDialog
{
   Q_OBJECT
public:
   explicit DeviceDriverViewGUI(QWidget *parent = nullptr);

private:
   Ui::DeviceDriverViewGUI ui;
   MD4040drive drive;
   SafeThread driveThread{this};
   Equip equipamento;
   QDateTime nextDate;
};

DeviceDriverViewGUI::DeviceDriverViewGUI(QWidget *parent) : QDialog(parent)
{
   ui.setupUi(this);
   //                                       vvvvvv -- gives the thread context
   connect(ui.pbTry, &QPushButton::clicked, &drive, [this]{
      Q_ASSERT(QThread::currentThread() == drive.thread()); // ensured by the thread context
      drive.requestMD4040drive(&equipamento, nextDate);
   });
   connect(&drive, &MD4040drive::end, this, [this](bool end){
      //...
   });
   drive.moveToThread(&driveThread);
   driveThread.start();
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   return app.exec();
}

#include "main.moc"
