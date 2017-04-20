#include <QtWidgets>
#include <QtNetwork>

class SocketSignaler : public QObject
{
   Q_OBJECT
   Q_SLOT void stateChanged(QAbstractSocket::SocketState state) {
      if (state == QAbstractSocket::UnconnectedState) emit unconnected();
      else emit busy();
      emit hasState(this->state());
   }
public:
   explicit SocketSignaler(QAbstractSocket * socket) : QObject(socket) {
      connect(socket, &QAbstractSocket::stateChanged, this, &SocketSignaler::stateChanged);
      connect(&(const QObject&)QObject(), &QObject::destroyed, this, // defer signal emission
              [=]{ emit stateChanged(socket->state()); }, Qt::QueuedConnection);
   }
   Q_SIGNAL void busy();
   Q_SIGNAL void unconnected();
   Q_SIGNAL void hasState(const QString &);
   QString state() const {
      switch (static_cast<QAbstractSocket*>(parent())->state()) {
      case QAbstractSocket::UnconnectedState: return "Disconnected";
      case QAbstractSocket::HostLookupState: return "Looking up host";
      case QAbstractSocket::ConnectingState: return "Connecting";
      case QAbstractSocket::ConnectedState: return "Connected";
      case QAbstractSocket::ClosingState: return "Closing";
      default: return {};
      }
   }
};

class Ui : public QWidget {
   Q_OBJECT
   Q_PROPERTY(bool busy WRITE setBusy)
   QVBoxLayout m_layout{this};
   QFormLayout m_form;
   QLineEdit m_target{"192.168.1.100"};
   QLineEdit m_message{"TRIG"};
   QLabel m_state;
   QDialogButtonBox m_box;
   QPushButton * const m_send = m_box.addButton("Send", QDialogButtonBox::AcceptRole);
   QPushButton * const m_cancel = m_box.addButton(QDialogButtonBox::Cancel);
   QMessageBox m_msgBox{this};
public:
   Ui() {
      m_form.addRow("Target Host", &m_target);
      m_form.addRow("Command", &m_message);
      m_layout.addLayout(&m_form);
      m_layout.addWidget(&m_state);
      m_layout.addWidget(&m_box);
      m_msgBox.setIcon(QMessageBox::Critical);
      connect(m_send, &QPushButton::clicked, this, &Ui::send);
      connect(m_cancel, &QPushButton::clicked, this, &Ui::cancel);
   }
   void setState(const QString & text) { m_state.setText(text); }
   QString target() const { return m_target.text(); }
   QString message() const { return m_message.text(); }
   void showError(const QString & text) {
      m_msgBox.setText(text);
      m_msgBox.show();
   }
   void setBusy(bool busy) {
      m_send->setEnabled(!busy);
      m_cancel->setEnabled(busy);
   }
   Q_SIGNAL void send();
   Q_SIGNAL void cancel();
};

int main(int argc, char *argv[])
{
   const int targetPort = 23;
   QApplication app{argc, argv};
   Ui ui;
   ui.show();

   QTcpSocket socket;
   SocketSignaler socketSig{&socket};
   QObject::connect(&socketSig, &SocketSignaler::hasState, &ui, &Ui::setState);

   QStateMachine machine;
   QState sReady{&machine};
   QState sBusy{&machine};
   sReady.assignProperty(&ui, "busy", false);
   sBusy.assignProperty(&ui, "busy", true);
   sReady.addTransition(&socketSig, &SocketSignaler::busy, &sBusy);
   sBusy.addTransition(&socketSig, &SocketSignaler::unconnected, &sReady);

   QObject::connect(&ui, &Ui::send, [&](){
      socket.connectToHost(ui.target(), targetPort);
   });
   QObject::connect(&ui, &Ui::cancel, [&](){ socket.abort(); });
   QObject::connect(&socket,
                    static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>
                    (&QAbstractSocket::error), [&]()
   {
      ui.showError(socket.errorString());
   });
   QObject::connect(&socket, &QAbstractSocket::connected, [&](){
      auto msg = ui.message().toLatin1();
      msg.append('\n');
      if (socket.write(msg) >= msg.size()) socket.close();
   });
   QObject::connect(&socket, &QAbstractSocket::bytesWritten, [&](){
      if (!socket.bytesToWrite()) socket.close();
   });

   machine.setInitialState(&sReady);
   machine.start();
   return app.exec();
}
#include "main.moc"
