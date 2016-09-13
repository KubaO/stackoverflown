#if 0

// https://github.com/KubaO/stackoverflown/tree/master/questions/async-sane-39396761
#include <QtWidgets>
#include <QtConcurrent>
#include <map>
#include <string>

struct Client {
    using result_type = std::map<std::string, std::map<std::string, std::string>>;
    result_type get_data() {
        QThread::sleep(5); // pretend to do some work
        return result_type();
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    Client::result_type exported_strings;
    QWidget centralWidget;
    QVBoxLayout layout{&centralWidget};
    QPushButton getDataButton{"Get Data"};
    QStatusBar statusBar;
    QTimer statusTimer;
    QString statusMessage;

    void setBusyStatus(const QString & status) {
        centralWidget.setEnabled(false);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        statusMessage = status;
        statusTimer.start(0);
    }
    void setNormalStatus(const QString & status) {
        centralWidget.setEnabled(true);
        QApplication::restoreOverrideCursor();
        statusBar.showMessage(status);
        statusTimer.stop();
    }
    Q_SLOT void on_getDataButtonClicked();
public:
    MainWindow() {
        setStatusBar(&statusBar);
        setCentralWidget(&centralWidget);
        layout.addWidget(&getDataButton);
        int n = 0;
        connect(&statusTimer, &QTimer::timeout, [=]() mutable {
            statusBar.showMessage(QStringLiteral("%1%2").arg(statusMessage).arg(QString{n+1, QChar{'.'}}));
            n = (n+1)%3;
            statusTimer.start(500);
        });
        connect(&getDataButton, &QPushButton::clicked, this, &MainWindow::on_getDataButtonClicked);
    }
};

void MainWindow::on_getDataButtonClicked()
{
    auto future = QtConcurrent::run([=]{
        Client client;
        return client.get_data();
    });
    auto watcher = new QFutureWatcher<Client::result_type>{this};
    connect(watcher, &QFutureWatcher<Client::result_type>::finished, this, [=]{
        exported_strings = std::move(watcher->result());
        watcher->deleteLater();
        setNormalStatus("All data has been retrieved!");
    });
    watcher->setFuture(future);
    setBusyStatus("Getting data");
}

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    MainWindow w;
    w.show();
    return app.exec();
}
#include "main.moc"

#endif

#if 1

#include <QtWidgets>
#include <future>
#include <map>
#include <string>

struct Client {
    using result_type = std::map<std::string, std::map<std::string, std::string>>;
    result_type get_data() {
        QThread::sleep(5); // pretend to do some work
        return result_type();
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    Client::result_type exported_strings;
    QWidget centralWidget;
    QVBoxLayout layout{&centralWidget};
    QPushButton getDataButton{"Get Data"};
    QStatusBar statusBar;
    QTimer statusTimer;
    QString statusMessage;

    std::future<Client::result_type> resultFuture;

    void setBusyStatus(const QString & status) {
        centralWidget.setEnabled(false);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        statusMessage = status;
        statusTimer.start(0);
    }
    void setNormalStatus(const QString & status) {
        centralWidget.setEnabled(true);
        QApplication::restoreOverrideCursor();
        statusBar.showMessage(status);
        statusTimer.stop();
    }
    Q_SLOT void on_getDataButtonClicked();
    Q_SIGNAL void hasResult();
public:
    MainWindow() {
        setStatusBar(&statusBar);
        setCentralWidget(&centralWidget);
        layout.addWidget(&getDataButton);
        int n = 0;
        connect(&statusTimer, &QTimer::timeout, [=]() mutable {
            statusBar.showMessage(QStringLiteral("%1%2").arg(statusMessage).arg(QString{n+1, QChar{'.'}}));
            n = (n+1)%3;
            statusTimer.start(500);
        });
        connect(&getDataButton, &QPushButton::clicked, this, &MainWindow::on_getDataButtonClicked);
    }
};

void MainWindow::on_getDataButtonClicked()
{
    connect(this, &MainWindow::hasResult, this, [this](){
        exported_strings = std::move(resultFuture.get());
        setNormalStatus("All data has been retrieved!");
    }, Qt::UniqueConnection);

    resultFuture = std::async(std::launch::async, [this]{
        Client client;
        auto result = client.get_data();
        emit hasResult();
        return result;
    });
    setBusyStatus("Getting data");
}

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    MainWindow w;
    w.show();
    return app.exec();
}
#include "main.moc"

#endif
