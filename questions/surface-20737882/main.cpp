#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QPointer>
#include <QBackingStore>
#include <QList>
#include <QBasicTimer>
#include <QImage>
#include <QPixmap>
#include <QTime>
#include <QDebug>

class WidgetMonitor : public QObject
{
    Q_OBJECT
    QSet<QWidget *> m_awake;
    QBasicTimer m_timer;
    int m_counter;
    void queue(QWidget * w) {
        m_awake << w->window();
        if (! m_timer.isActive()) m_timer.start(0, this);
    }
    bool eventFilter(QObject * obj, QEvent * ev) {
        switch (ev->type()) {
        case QEvent::Paint: {
            if (! obj->isWidgetType()) break;
            queue(static_cast<QWidget*>(obj));
            break;
        }
        case QEvent::ChildAdded: {
            if (obj->isWidgetType()) obj->installEventFilter(this);
            break;
        }
        default: break;
        }
        return false;
    }
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() != m_timer.timerId()) return;
        qDebug() << "painting: " << m_counter++ << m_awake;
        foreach (QWidget * w, m_awake) {
            QImage * img = dynamic_cast<QImage*>(w->backingStore()->paintDevice());
            if (img) emit newContents(img, w);
        }
        m_awake.clear();
        m_timer.stop();
    }
    Q_SLOT void windowDestroyed(QObject * obj) {
        if (! obj->isWidgetType()) return;
        m_awake.remove(static_cast<QWidget*>(obj));
    }
public:
    explicit WidgetMonitor(QObject * parent = 0) : QObject(parent), m_counter(0) {}
    explicit WidgetMonitor(QWidget * w, QObject * parent = 0) : QObject(parent), m_counter(0) {
        monitor(w);
    }
    Q_SLOT void monitor(QWidget * w) {
        w = w->window();
        connect(w, &QObject::destroyed, this, &WidgetMonitor::windowDestroyed);
        w->installEventFilter(this);
        foreach (QObject * obj, w->children()) {
            if (obj->isWidgetType()) obj->installEventFilter(this);
        }
        queue(w);
    }
    Q_SLOT void unMonitor(QWidget * w) {
        w = w->window();
        disconnect(w, &QObject::destroyed, this, &WidgetMonitor::windowDestroyed);
        m_awake.remove(w);
        w->removeEventFilter(this);
        foreach (QObject * obj, w->children()) {
            if (obj->isWidgetType()) obj->removeEventFilter(this);
        }
    }
    Q_SIGNAL void newContents(const QImage *, QWidget * w);
};

class TestWidget : public QWidget {
    QLabel * m_time;
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() != m_timer.timerId()) return;
        m_time->setText(QTime::currentTime().toString());
    }
public:
    explicit TestWidget(QWidget * parent = 0) : QWidget(parent), m_time(new QLabel) {
        QGridLayout * lay = new QGridLayout(this);
        lay->addWidget(m_time, 0, 0);
        lay->addWidget(new QLabel("Static Label"), 1, 0);
        lay->addWidget(new QPushButton("A Button"), 2, 0);
        m_timer.start(1000, this);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TestWidget src;
    QWidget dst;
    QGridLayout * dl = new QGridLayout(&dst);
    QLabel * dstLabel = new QLabel;
    dstLabel->setFrameShape(QFrame::Box);
    dl->addWidget(new QLabel("Destination"), 0, 0);
    dl->addWidget(dstLabel);
    src.show();
    dst.show();

    WidgetMonitor mon(&src);
    QObject::connect(&mon, &WidgetMonitor::newContents, [=](const QImage * img){
        dstLabel->resize(img->size());
        dstLabel->setPixmap(QPixmap::fromImage(*img));

    });
    return a.exec();
}

#include "main.moc"
