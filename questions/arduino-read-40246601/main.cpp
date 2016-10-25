#if 0

// https://github.com/KubaO/stackoverflown/tree/master/questions/arduino-read-40246601
#include <QtSerialPort>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication a{argc, argv};
    QTextStream in{stdin};
    QTextStream out{stdout};

    QSerialPort port;
    port.setPortName("COM6");
    port.setBaudRate(9600);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);

    if (!port.open(QSerialPort::ReadWrite)) {
        out << "Error opening serial port: " << port.errorString() << endl;
        return 1;
    }

    while(true)
    {
        out << "> ";
        auto cmd = in.readLine().toLatin1();
        if (cmd.length() < 1)
            continue;

        port.write(cmd);

        while (!port.canReadLine())
            port.waitForReadyRead(-1);

        while (port.canReadLine())
            out << "< " << port.readLine(); // lines are already terminated
    }
}

#endif

#if 1

#include <QtSerialPort>
#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication app{argc, argv};
    QWidget ui;
    QFormLayout layout{&ui};
    QLineEdit portName{"COM6"};
    QTextBrowser term;
    QLineEdit command;
    QPushButton open{"Open"};
    layout.addRow("Port", &portName);
    layout.addRow(&term);
    layout.addRow("Command:", &command);
    layout.addRow(&open);
    ui.show();

    QSerialPort port;
    port.setBaudRate(9600);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);

    QObject::connect(&open, &QPushButton::clicked, &port, [&]{
        port.setPortName(portName.text());
        if (port.open(QSerialPort::ReadWrite)) return;
        term.append(QStringLiteral("* Error opening serial port: %1").arg(port.errorString()));
    });

    QObject::connect(&command, &QLineEdit::returnPressed, &port, [&]{
        term.append(QStringLiteral("> %1").arg(command.text()));
        port.write(command.text().toLatin1());
    });

    QObject::connect(&port, &QIODevice::readyRead, &term, [&]{
        if (!port.canReadLine()) return;
        while (port.canReadLine())
            term.append(QStringLiteral("< %1").arg(QString::fromLatin1(port.readLine())));
    });
    return app.exec();
}

#endif
