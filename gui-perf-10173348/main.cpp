#include <QElapsedTimer>
#include <QPaintEvent>
#include <QBasicTimer>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QWidget>
#include <QDebug>

static inline int rand(int range) { return (double(qrand()) * range) / RAND_MAX; }

class Widget : public QWidget
{
    float fps;
    qint64 lastTime;
    QPixmap pixmap;
    QBasicTimer timer;
    QElapsedTimer elapsed;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == timer.timerId()) update();
    }
    void paintEvent(QPaintEvent * ev) {
        qint64 time = elapsed.elapsed();
        qint64 delta = time - lastTime;
        lastTime = time;
        if (delta > 0) {
            const float weight(0.05);
            fps = (1.0-weight)*fps + weight*(1E3/delta);

            if (pixmap.size() != size()) {
                pixmap = QPixmap(size());
                pixmap.fill(Qt::black);
            }
            int dy = qMin((int)delta, pixmap.height());
            pixmap.scroll(0, dy, pixmap.rect());
            QPainter pp(&pixmap);
            pp.fillRect(0, 0, pixmap.width(), dy, Qt::black);
            for(int i = 0; i < 30; ++i){
                int x = rand(pixmap.width());
                pp.fillRect(x, 0, 3, dy, Qt::green);
            }
        }
        QPainter p(this);
        p.drawPixmap(ev->rect(), pixmap, ev->rect());
        p.setPen(Qt::yellow);
        p.fillRect(0, 0, 100, 50, Qt::black);
        p.drawText(rect(), QString("FPS: %1").arg(fps, 0, 'f', 0));
    }

public:
    explicit Widget(QWidget *parent = 0) : QWidget(parent), fps(0), lastTime(0), pixmap(size())
    {
        timer.start(1000/60, this);
        elapsed.start();
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
