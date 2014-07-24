#include <QApplication>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QThread>
#include <QtConcurrent>

class Thread : public QThread {
    using QThread::run;
public:
    ~Thread() { quit(); wait(); }
};

class Loader : public QObject
{
    Q_OBJECT
public:
    explicit Loader(QObject *parent = 0) : QObject(parent) {}
    Q_SIGNAL void imageLoaded(QString, const QImage &);
    Q_SLOT void loadImage(const QString& fichier) {
        QImage img(fichier);
        if (! img.isNull()) emit imageLoaded(fichier, img);
    }
};

class MainWindow : public QWidget
{
    Q_OBJECT
    Loader m_loader;
    Thread m_loaderThread;
    QGridLayout m_layout;
    QPushButton m_open;
    QScrollArea m_view;
    QWidget m_content;
    int m_width;
    bool m_threadImpl;
    Q_SIGNAL void loadImage(const QString &);
    Q_SIGNAL void imageLoaded(const QString &, const QImage & img);
    Q_SLOT void imageAvailable(const QString &, const QImage & img) {
        int spacing = 20;
        if (m_width) m_width += spacing;
        QLabel * lab = new QLabel(&m_content);
        lab->setFixedSize(img.width(), img.height());
        lab->setGeometry(m_width, 0, img.width(), img.height());
        lab->setPixmap(QPixmap::fromImage(img));
        lab->show();
        m_width += img.width();
        m_content.setMinimumWidth(m_width);
        m_content.setMinimumHeight(qMax(m_content.minimumHeight(), img.height()));
    }
    Q_SLOT void open() {
        QFileDialog * dialog = new QFileDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        if (m_threadImpl)
            connect(dialog, SIGNAL(fileSelected(QString)), SIGNAL(loadImage(QString)));
        else
            connect(dialog, &QFileDialog::fileSelected, [this](const QString & fichier){
                QtConcurrent::run([this, fichier]{
                    QImage img(fichier);
                    if (! img.isNull()) emit this->imageLoaded(fichier, img);
                });
            });
        m_threadImpl = !m_threadImpl;
    }
public:
    explicit MainWindow(QWidget *parent = 0) : QWidget(parent), m_layout(this), m_open("Open"),
        m_width(0), m_threadImpl(true) {
        m_layout.addWidget(&m_open);
        m_layout.addWidget(&m_view);
        m_view.setWidget(&m_content);
        m_loader.moveToThread(&m_loaderThread);
        m_loaderThread.start();
        connect(&m_open, SIGNAL(clicked()), SLOT(open()));
        connect(this, SIGNAL(loadImage(QString)), &m_loader, SLOT(loadImage(QString)));
        connect(this, SIGNAL(imageLoaded(QString,QImage)), SLOT(imageAvailable(QString,QImage)));
        connect(&m_loader, SIGNAL(imageLoaded(QString,QImage)), SLOT(imageAvailable(QString,QImage)));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "main.moc"
