// https://github.com/KubaO/stackoverflown/tree/master/questions/async-file-io-39226814
#include <QtWidgets>
#include <QtConcurrent>

class AData : public QObject
{
    Q_OBJECT
    QFile m_file;
    QThreadPool * m_ioPool = QThreadPool::globalInstance();
public:
    explicit AData(QObject * parent = nullptr) : QObject{parent} {
        connect(this, &AData::save, [=](const QByteArray & data, qint64 pos){
            QtConcurrent::run(m_ioPool, [=]{
                m_file.seek(pos);
                m_file.write(data);
            });
        });
        connect(this, &AData::load, [=](qint64 pos, qint64 len){
           QtConcurrent::run(m_ioPool, [=]() mutable{
               m_file.seek(pos);
               if (len == -1) len = m_file.size();
               auto data = m_file.read(len);
               emit loaded(data, pos);
           });
        });
    }
    bool open(const QString & name) {
        m_file.setFileName(name);
        return m_file.open(QIODevice::ReadWrite);
    }
    void setPool(QThreadPool * pool) { m_ioPool = pool; }
    Q_SIGNAL void save(const QByteArray &data, qint64 pos = 0) const;
    Q_SIGNAL void load(qint64 pos, qint64 len) const;
    Q_SIGNAL void loaded(const QByteArray &data, qint64 pos) const;
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QThreadPool ioPool;
    AData data;
    data.setPool(&ioPool);
    data.open("myfile");

    QWidget ui;
    QGridLayout layout{&ui};
    QTextEdit text;
    QPushButton load{"Load"};
    QPushButton save{"Save"};
    QPushButton clear{"Clear"};
    layout.addWidget(&text, 0, 0, 1, 2);
    layout.addWidget(&load, 1, 0);
    layout.addWidget(&save, 1, 1);
    layout.addWidget(&clear, 2, 0, 1, 2);
    ui.show();

    using Q = QObject;
    Q::connect(&load, &QPushButton::clicked, &data, [&]{
        data.load(0, -1);
    });
    Q::connect(&data, &AData::loaded, &app, [&](const QByteArray & data, qint64){
       text.setPlainText(QString::fromUtf8(data));
    });
    Q::connect(&save, &QPushButton::clicked, &data, [&]{
        data.save(text.document()->toPlainText().toUtf8());
    });
    Q::connect(&clear, &QPushButton::clicked, &text, &QTextEdit::clear);

    return app.exec();
}
#include "main.moc"
