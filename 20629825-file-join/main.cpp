#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QFormLayout>
#include <QMetaObject>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QApplication>

class Worker : public QObject {
    Q_OBJECT
    Q_PROPERTY(int fileIndex READ fileIndex WRITE setFileIndex)
    Q_PROPERTY(QString sourceFileName READ sourceFileName)
    Q_PROPERTY(QString targetFileName READ targetFileName WRITE setTargetFileName)
    int m_fileIndex;
    QString m_targetFileName;
public:
    Worker(int fileIndex, QString targetFileName, QObject * parent = 0) :
        QObject(parent), m_fileIndex(fileIndex), m_targetFileName(targetFileName) {}
    explicit Worker(QObject * parent = 0) : QObject(parent), m_fileIndex(0) {}
    Q_SIGNAL void filesJoined(bool OK);
    Q_SLOT void joinFiles() {
        QFile src(sourceFileName());
        QFile dst(m_targetFileName);
        if (! src.open(QIODevice::ReadOnly)
            || ! dst.open(QIODevice::Append))
        {
            emit filesJoined(false);
            return;
        }
        while (! src.atEnd()) {
            dst.write(src.read(16384));
        }
        emit filesJoined(true);
    }
    int fileIndex() const { return m_fileIndex; }
    void setFileIndex(int i) { m_fileIndex = i; }
    QString sourceFileName() const { return QString("test%1.txt").arg(m_fileIndex); }
    QString targetFileName() const { return m_targetFileName; }
    void setTargetFileName(const QString & n) { m_targetFileName = n; }
};

class Window : public QWidget {
    Q_OBJECT
    QThread * m_thread;
    QSpinBox * m_index;
    QLineEdit * m_target;
    QPlainTextEdit * m_log;
    Q_SLOT void on_pushButton_clicked() {
        Worker * w = new Worker;
        w->setFileIndex(m_index->value());
        w->setTargetFileName(m_target->text());
        w->moveToThread(m_thread);
        connect(w, SIGNAL(filesJoined(bool)), SLOT(onJoined(bool)));
        QMetaObject::invokeMethod(w, "joinFiles");
    }
    Q_SLOT void onJoined(bool ok) {
        const Worker * w = qobject_cast<const Worker*>(sender());
        m_log->appendPlainText(QString("%1 %2 to %3")
                               .arg(ok ? "Successfully joined" : "Couldn't join")
                               .arg(w->sourceFileName()).arg(w->targetFileName()));
        sender()->deleteLater();
    }
public:
    Window(QWidget * parent = 0) :
        QWidget(parent),
        m_thread(new QThread(this)),
        m_index(new QSpinBox),
        m_target(new QLineEdit),
        m_log(new QPlainTextEdit)
    {
        QFormLayout * layout = new QFormLayout(this);
        QPushButton * button = new QPushButton("Join Files");
        button->setObjectName("pushButton");
        layout->addRow("File Index", m_index);
        layout->addRow("Append to File Name", m_target);
        layout->addRow(button);
        layout->addRow(m_log);
        QMetaObject::connectSlotsByName(this);
        m_thread->start();
        m_log->appendPlainText(QString("Current directory: %1").arg(QDir::currentPath()));
    }
    ~Window() {
        m_thread->exit();
        m_thread->wait();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
