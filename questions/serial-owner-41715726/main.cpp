// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-owner-41715726
#include <QtWidgets>
#include <QtSerialPort>

class Operations1 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_send{"Send"};
    QPointer<QSerialPort> m_serial;
public:
    Operations1() {
        m_layout.addWidget(&m_send);
        connect(&m_send, &QPushButton::clicked, this, &Operations1::sendRequest);
    }
    void sendRequest() {
        QByteArray request;
        QDataStream ds(&request, QIODevice::WriteOnly);
        ds << qint32(44);
        m_serial->write(request);
    }
    void setSerial(QSerialPort * port) {
        m_serial = port;
    }
};

class MainWindow1 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_open{"Open"};
    QSerialPort m_serial;
    QScopedPointer<Operations1> m_operations;
    Operations1 * operations() {
        if (!m_operations)
            m_operations.reset(new Operations1);
        return m_operations.data();
    }
public:
    MainWindow1() {
        m_layout.addWidget(&m_open);
        connect(&m_open, &QPushButton::clicked, this, &MainWindow1::open);
    }
    void open() {
        m_serial.setBaudRate(38400);
        m_serial.setPortName("/dev/tty.usbserial-PX9A3C3B");
        if (!m_serial.open(QIODevice::ReadWrite))
            return;
        operations()->show();
        operations()->setSerial(&m_serial);
    }
};

int main1(int argc, char ** argv) {
    QApplication app{argc, argv};
    MainWindow1 ui;
    ui.show();
    return app.exec();
}

//

class Controller2 : public QObject {
    Q_OBJECT
    QSerialPort m_port;
public:
    Controller2(QObject * parent = nullptr) : QObject{parent} {
        connect(&m_port, &QIODevice::bytesWritten, this, [this]{
            if (m_port.bytesToWrite() == 0)
                emit allDataSent();
        });
    }
    Q_SLOT void open() {
        m_port.setBaudRate(38400);
        m_port.setPortName("/dev/tty.usbserial-PX9A3C3B");
        if (!m_port.open(QIODevice::ReadWrite))
            return;
        emit opened();
    }
    Q_SIGNAL void opened();
    Q_SLOT void sendRequest() {
        QByteArray request;
        QDataStream ds(&request, QIODevice::WriteOnly);
        ds << qint32(44);
        m_port.write(request);
    }
    Q_SIGNAL void allDataSent();
};

class Operations2 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_send{"Send"};
    QPointer<Controller2> m_ctl;
public:
    Operations2(Controller2 * ctl, QWidget * parent = nullptr) :
        QWidget{parent},
        m_ctl{ctl}
    {
        m_layout.addWidget(&m_send);
        connect(&m_send, &QPushButton::clicked, m_ctl, &Controller2::sendRequest);
    }
};

class MainWindow2 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_open{"Open"};
    QPointer<Controller2> m_ctl;
    QScopedPointer<Operations2> m_operations;
    Operations2 * operations() {
        if (!m_operations)
            m_operations.reset(new Operations2{m_ctl});
        return m_operations.data();
    }
public:
    MainWindow2(Controller2 * ctl, QWidget * parent = nullptr) :
        QWidget{parent},
        m_ctl{ctl}
    {
        m_layout.addWidget(&m_open);
        connect(&m_open, &QPushButton::clicked, m_ctl, &Controller2::open);
        connect(m_ctl, &Controller2::opened, this, [this]{
            operations()->show();
        });
    }
};

int main2(int argc, char ** argv) {
    QApplication app{argc, argv};
    Controller2 controller;
    MainWindow2 ui(&controller);
    ui.show();
    return app.exec();
}

//

class Controller3 : public QObject {
    Q_OBJECT
    QSerialPort m_port;
    static Controller3 * instance(bool assign, Controller3 * newInstance = nullptr) {
        static Controller3 * instance;
        if (assign)
            instance = newInstance;
        return instance;
    }
public:
    Controller3(QObject * parent = nullptr) : QObject{parent} {
        connect(&m_port, &QIODevice::bytesWritten, this, [this]{
            if (m_port.bytesToWrite() == 0)
                emit allDataSent();
        });
        instance(true, this);
    }
    ~Controller3() {
        instance(true);
    }
    Q_SLOT void open() {
        m_port.setBaudRate(38400);
        m_port.setPortName("/dev/tty.usbserial-PX9A3C3B");
        if (!m_port.open(QIODevice::ReadWrite))
            return;
        emit opened();
    }
    Q_SIGNAL void opened();
    Q_SLOT void sendRequest() {
        QByteArray request;
        QDataStream ds(&request, QIODevice::WriteOnly);
        ds << qint32(44);
        m_port.write(request);
    }
    Q_SIGNAL void allDataSent();
    static Controller3 * instance() {
        return instance(false);
    }
};

class Operations3 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_send{"Send"};
public:
    Operations3(QWidget * parent = nullptr) : QWidget{parent}
    {
        m_layout.addWidget(&m_send);
        connect(&m_send, &QPushButton::clicked, Controller3::instance(), &Controller3::sendRequest);
    }
};

class MainWindow3 : public QWidget {
    Q_OBJECT
    QVBoxLayout m_layout{this};
    QPushButton m_open{"Open"};
    QScopedPointer<Operations3> m_operations;
    Operations3 * operations() {
        if (!m_operations)
            m_operations.reset(new Operations3);
        return m_operations.data();
    }
public:
    MainWindow3(QWidget * parent = nullptr) : QWidget{parent}
    {
        m_layout.addWidget(&m_open);
        connect(&m_open, &QPushButton::clicked, Controller3::instance(), &Controller3::open);
        connect(Controller3::instance(), &Controller3::opened, this, [this]{
            operations()->show();
        });
    }
};

int main3(int argc, char ** argv) {
    QApplication app{argc, argv};
    Controller3 controller;
    MainWindow3 ui;
    ui.show();
    return app.exec();
}

//

int main(int argc, char **argv) { return main3(argc, argv); }

#include "main.moc"

