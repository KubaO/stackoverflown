#include <QApplication>
#include <QAbstractAnimation>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QDebug>

class PathAnimation : public QAbstractAnimation {
    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    QPainterPath m_path;
    int m_duration;
    QVector<QPointF> m_cache;
    QGraphicsItem * m_target;
    int m_hits, m_misses;
public:
    PathAnimation(const QPainterPath & path, QObject * parent = 0) :
        QAbstractAnimation(parent), m_path(path), m_duration(1000), m_cache(m_duration), m_target(0), m_hits(0), m_misses(0) {}
    ~PathAnimation() { qDebug() << m_hits << m_misses; }
    int duration() const { return m_duration; }
    void setDuration(int duration) {
        if (duration == 0 || duration == m_duration) return;
        m_duration = duration;
        m_cache.clear();
        m_cache.resize(m_duration);
    }
    void setTarget(QGraphicsItem * target) {
        m_target = target;
    }
    void updateCurrentTime(int ms) {
        QPointF point = m_cache.at(ms);
        if (! point.isNull()) {
            ++ m_hits;
        } else {
            point = m_path.pointAtPercent(qreal(ms) / m_duration);
            m_cache[ms] = point;
            ++ m_misses;
        }
        if (m_target) m_target->setPos(point);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsEllipseItem * item = new QGraphicsEllipseItem(-5, -5, 10, 10);
    item->setPen(QPen(Qt::red, 2));
    item->setBrush(Qt::lightGray);

    QPainterPath path;
    path.addEllipse(0, 0, 100, 100);
    PathAnimation animation(path);
    animation.setTarget(item);

    QGraphicsScene scene;
    scene.addItem(item);
    QGraphicsView view(&scene);
    view.setSceneRect(-50, -50, 200, 200);

    animation.setLoopCount(-1);
    animation.start();
    view.show();

    return a.exec();
}

#include "main.moc"
