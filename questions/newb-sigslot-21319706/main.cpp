#include <QApplication>
#include <QTimer>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

class MessageLogCommand : public QWidget
{
    Q_OBJECT
    QLabel homeLabel;
    QPlainTextEdit messageLog;
public:
    explicit MessageLogCommand(QWidget *parent = 0) :
        QWidget(parent),
        homeLabel("GSS Message Log")
    {
        QGridLayout *layout = new QGridLayout(this);
        layout->addWidget(&homeLabel, 0, 0);
        layout->addWidget(&messageLog, 1, 0);
    }
    Q_SLOT void updateWidgets(const QString &text)
    {
        messageLog.appendPlainText(text);
    }
};

class HomeCommand : public QWidget
{
    Q_OBJECT
public:
    explicit HomeCommand(QWidget *parent = 0) : QWidget(parent)
    {
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(run()));
        timer->start(1000);
    }
    Q_SIGNAL void textChanged(const QString &text);
    Q_SLOT void run() {
        Q_EMIT textChanged("ZOMG");
    }
};

int main(int argc, char ** argv) {
    QApplication app(argc, argv);
    MessageLogCommand s;
    HomeCommand m;
    s.show();
    QObject::connect(&m, SIGNAL(textChanged(QString)),
                     &s, SLOT(updateWidgets(QString)));
    return app.exec();
}

#include "main.moc"
