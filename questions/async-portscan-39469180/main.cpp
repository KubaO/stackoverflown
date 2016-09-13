// https://github.com/KubaO/stackoverflown/tree/master/questions/async-portscan-39469180
#include <QtWidgets>
#include <QtConcurrent>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

class Scanner : public QObject {
    Q_OBJECT
    bool running = false, stop = false;
    int open = 0, closed = 0, total = 0;
    void scan() {
        running = true;
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        for (int i = 1; i < 65536 && !stop; ++i) {
            auto s = socket(AF_INET, SOCK_STREAM, 0);
            addr.sin_port = htons(i);
            auto con = ::connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr));
            emit hasResult(i, con == 0);
            con == 0 ? ++open : ++closed;
            ++total;
            ::close(s);
        }
        emit done();
        running = false;
    }
public:
    ~Scanner() {
        stop = true;
        while (running);
    }
    Q_SIGNAL void hasResult(int port, bool open);
    Q_SIGNAL void done();
    Q_SLOT void start() {
        QtConcurrent::run(this, &Scanner::scan);
    }
};

int main(int argc, char ** argv) {
    using Q = QObject;
    QApplication app{argc, argv};
    QWidget ui;
    QVBoxLayout layout{&ui};
    QTextBrowser log;
    QProgressBar bar;
    QPushButton scan{"Scan localhost"};
    layout.addWidget(&log);
    layout.addWidget(&bar);
    layout.addWidget(&scan);
    bar.setRange(1, 65535);
    ui.show();

    Scanner scanner;
    Q::connect(&scan, &QPushButton::clicked, &scanner, [&]{
        scan.setEnabled(false);
        scanner.start();
    });
    Q::connect(&scanner, &Scanner::hasResult, &log, [&](int port, bool isOpen){
        bar.setValue(port);
        if (!isOpen) return;
        auto color = isOpen ? QStringLiteral("green") : QStringLiteral("red");
        auto state = isOpen ? QStringLiteral("open") : QStringLiteral("closed");
        log.append(QStringLiteral("<font color=\"%1\">Port %2 is %3.</font><br/>").
                   arg(color).arg(port).arg(state));
    });
    Q::connect(&scanner, &Scanner::done, &scan, [&]{
        bar.reset();
        scan.setEnabled(true);
    });
    return app.exec();
}
#include "main.moc"
