#include <QLabel>
#include <QHBoxLayout>
#include <QBasicTimer>
#include <QProcess>
#include <QApplication>

class Widget : public QWidget {
    Q_OBJECT
    QHBoxLayout m_layout;
    QLabel m_label;
    QBasicTimer m_timer;
    QProcess m_process;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) txMessage();
    }
    void txMessage() {
        m_timer.stop();
        m_process.start("netstat", QStringList() << "-i", QProcess::ReadOnly);
    }
    Q_SLOT void finished(int rc) {
        startTimer();
        if (rc != 0) {
            m_label.setText("Error");
        } else {
            QString output = QString::fromLocal8Bit(m_process.readAll());
            QStringList lines = output.split('\n', QString::SkipEmptyParts);
            foreach (QString line, lines) {
                if (!line.contains("en0")) continue;
                QStringList args = line.split(' ', QString::SkipEmptyParts);
                if (args.count() >= 3) {
                    m_label.setText(args.at(3));
                    return;
                }
            }
        }
        m_label.setText("...");
    }
    void startTimer() {
#if QT_VERSION>=QT_VERSION_CHECK(5,0,0)
        m_timer.start(1000, Qt::CoarseTimer, this);
#else
        m_timer.start(1000, this);
#endif
    }
public:
    Widget(QWidget * parent = 0) : QWidget(parent), m_layout(this), m_label("...") {
        m_layout.addWidget(&m_label);
        startTimer();
        connect(&m_process, SIGNAL(finished(int)), SLOT(finished(int)));
    }
};

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}

#include "main.moc"
