#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QStateMachine>
#include <QTcpSocket>

class SocketSignaler : public QObject
{
    Q_OBJECT
    Q_SLOT void stateChanged(QAbstractSocket::SocketState state) {
        if (state == QAbstractSocket::UnconnectedState) { emit unconnected(); }
        else { emit busy(); }
        emit hasState(this->state());
    }
public:
    explicit SocketSignaler(QAbstractSocket * socket) : QObject(socket) {
        connect(socket, &QAbstractSocket::stateChanged, this, &SocketSignaler::stateChanged);
    }
    Q_SIGNAL void busy();
    Q_SIGNAL void unconnected();
    Q_SIGNAL void hasState(const QString &);
    QString state() {
        switch (static_cast<QAbstractSocket*>(parent())->state()) {
        case QAbstractSocket::UnconnectedState: return "Disconnected";
        case QAbstractSocket::HostLookupState: return "Looking up host";
        case QAbstractSocket::ConnectingState: return "Connecting";
        case QAbstractSocket::ConnectedState: return "Connected";
        case QAbstractSocket::ClosingState: return "Closing";
        default: return QString();
        }
    }
};

int main(int argc, char *argv[])
{
    const int targetPort = 23;
    QApplication a(argc, argv);
#if defined(Q_OS_MACX)
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif
    QWidget w;
    QVBoxLayout * l = new QVBoxLayout(&w);
    QFormLayout * f = new QFormLayout;
    QLineEdit * target = new QLineEdit;
    QLineEdit * message = new QLineEdit;
    target->setText("192.168.1.100");
    message->setText("TRIG");
    f->addRow("Target Host", target);
    f->addRow("Command", message);
    l->addLayout(f);
    QLabel * state = new QLabel;
    l->addWidget(state);
    QDialogButtonBox * box = new QDialogButtonBox;
    l->addWidget(box);
    QPushButton * send = box->addButton("Send", QDialogButtonBox::AcceptRole);
    QPushButton * cancel = box->addButton(QDialogButtonBox::Cancel);
    w.show();

    QMessageBox * mbox = new QMessageBox(&w);
    mbox->setIcon(QMessageBox::Critical);

    QTcpSocket * socket = new QTcpSocket(&a);
    SocketSignaler * socketSig = new SocketSignaler(socket);
    state->setText(socketSig->state());
    QObject::connect(socketSig, &SocketSignaler::hasState, state, &QLabel::setText);

    QStateMachine machine;
    QState * sReady = new QState(&machine);
    QState * sBusy = new QState(&machine);
    sReady->assignProperty(send, "enabled", true);
    sReady->assignProperty(cancel, "enabled", false);
    sBusy->assignProperty(send, "enabled", false);
    sBusy->assignProperty(cancel, "enabled", true);
    sReady->addTransition(socketSig, SIGNAL(busy()), sBusy);
    sBusy->addTransition(socketSig, SIGNAL(unconnected()), sReady);

    QObject::connect(send, &QPushButton::clicked, [=](){
        socket->connectToHost(target->text(), targetPort);
    });
    QObject::connect(cancel, &QPushButton::clicked, [=](){ socket->abort(); });
    QObject::connect(socket,
        static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>
                    (&QAbstractSocket::error), [=]()
    {
        mbox->setText(socket->errorString());
        mbox->show();
    });
    QObject::connect(socket, &QAbstractSocket::connected, [=](){
        const QByteArray msg(message->text().toLatin1());
        if (socket->write(msg) >= msg.size()) socket->close();
    });
    QObject::connect(socket, &QAbstractSocket::bytesWritten, [=](){
        if (!socket->bytesToWrite()) socket->close();
    });

    machine.setInitialState(sReady);
    machine.start();
    return a.exec();
}

#include "main.moc"
