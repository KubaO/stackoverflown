// https://github.com/KubaO/stackoverflown/tree/master/questions/image-loader-24853687
#include <QtWidgets>
#include <QtConcurrent>

class Thread final : public QThread {
public:
    ~Thread() { quit(); wait(); }
};

class Loader : public QObject
{
    Q_OBJECT
public:
    explicit Loader(QObject *parent = nullptr) : QObject(parent) {}
    Q_SIGNAL void imageLoaded(const QString &, const QImage &);
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
    QGridLayout m_layout{this};
    QPushButton m_open{"Open"};
    QScrollArea m_view;
    QWidget m_content;
    int m_width{};
    bool m_threadImpl = true;
    Q_SIGNAL void loadImage(const QString &);
    Q_SIGNAL void imageLoaded(const QString &, const QImage & img);
    Q_SLOT void imageAvailable(const QString &, const QImage & img) {
        int spacing = 20;
        if (m_width) m_width += spacing;
        auto lab = new QLabel(&m_content);
        lab->setFixedSize(img.width(), img.height());
        lab->setGeometry(m_width, 0, img.width(), img.height());
        lab->setPixmap(QPixmap::fromImage(img));
        lab->show();
        m_width += img.width();
        m_content.setMinimumWidth(m_width);
        m_content.setMinimumHeight(qMax(m_content.minimumHeight(), img.height()));
    }
    Q_SLOT void open() {
        auto dialog = new QFileDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        if (m_threadImpl)
            connect(dialog, &QFileDialog::fileSelected, this, &MainWindow::loadImage);
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
    explicit MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        m_layout.addWidget(&m_open);
        m_layout.addWidget(&m_view);
        m_view.setWidget(&m_content);
        m_loader.moveToThread(&m_loaderThread);
        m_loaderThread.start();
        connect(&m_open, &QPushButton::clicked, this, &MainWindow::open);
        connect(this, &MainWindow::loadImage, &m_loader, &Loader::loadImage);
        connect(this, &MainWindow::imageLoaded, this, &MainWindow::imageAvailable);
        connect(&m_loader, &Loader::imageLoaded, this, &MainWindow::imageAvailable);
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
