    //main.cpp
    #include <QMainWindow>
    #include <QVBoxLayout>
    #include <QStatusBar>
    #include <QLabel>
    #include <QThread>
    #include <QPainter>
    #include <QImage>
    #include <QApplication>
    #include <QBasicTimer>
    #include <QPushButton>

    class DrawThing : public QObject {
        Q_OBJECT
        int m_ctr;
        QBasicTimer t;
        void timerEvent(QTimerEvent * ev) {
            if (ev->timerId() != t.timerId()) return;
            QImage img(128, 128, QImage::Format_RGB32);
            QPainter p(&img);
            p.translate(img.size().width()/2, img.size().height()/2);
            p.scale(img.size().width()/2, img.size().height()/2);
            p.eraseRect(-1, -1, 2, 2);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::black, 0.05));
            p.drawEllipse(QPointF(), 0.9, 0.9);
            p.rotate(m_ctr*360/12);
            p.setPen(QPen(Qt::red, 0.1));
            p.drawLine(0, 0, 0, 1);
            m_ctr = (m_ctr + 1) % 12;
            emit newImage(img);
        }
    public:
        explicit DrawThing(QObject *parent = 0) : QObject(parent), m_ctr(0) { t.start(1000, this); }
        Q_SIGNAL void newImage(const QImage &);
    };

    class MainWindow : public QMainWindow {
        Q_OBJECT
        QLabel *m_label;
    public:
        explicit MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0) : QMainWindow(parent, flags) {
            QWidget * cw = new QWidget;
            QTabWidget * tw = new QTabWidget();
            QVBoxLayout * l = new QVBoxLayout(cw);
            l->addWidget(tw);
            l->addWidget(m_label = new QLabel("Label"));
            setCentralWidget(cw);
            QPushButton * pb = new QPushButton("Toggle Status Bar");
            tw->addTab(pb, "Tab 1");
            connect(pb, SIGNAL(clicked()), SLOT(toggleStatusBar()));
            statusBar()->showMessage("The Status Bar");
        }
        Q_SLOT void setImage(const QImage & img) {
            m_label->setPixmap(QPixmap::fromImage(img));
        }
        Q_SLOT void toggleStatusBar() {
            statusBar()->setHidden(!statusBar()->isHidden());
        }
    };

    int main(int argc, char *argv[])
    {
        QApplication a(argc, argv);
        QThread t;
        DrawThing thing;
        MainWindow w;
        thing.moveToThread(&t);
        t.start();
        w.connect(&thing, SIGNAL(newImage(QImage)), SLOT(setImage(QImage)));
        w.show();
        t.connect(&a, SIGNAL(aboutToQuit()), SLOT(quit()));
        int rc = a.exec();
        t.wait();
        return rc;
    }

    #include "main.moc"
