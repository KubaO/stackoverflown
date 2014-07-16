#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

class ClearBack : public QWidget
{
    Q_OBJECT
    QRect m_currentRegion, m_lastRegion;
public:
    explicit ClearBack(const QPoint &startingPos) :
        m_currentRegion(startingPos, startingPos)
    {
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        showMaximized();
    }
    Q_SIGNAL void regionSelected(const QRect &);
protected:
    void paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::transparent, 3));
        painter.drawRect(m_lastRegion);
        m_lastRegion = m_currentRegion;
        painter.setPen(Qt::black);
        painter.drawRect(m_currentRegion);
    }
    void mouseMoveEvent(QMouseEvent *event) {
        m_currentRegion.setBottomRight(event->globalPos());
        update();
    }
    void mouseReleaseEvent(QMouseEvent *) {
        emit regionSelected(m_currentRegion);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClearBack back(QPoint(200,200));
    a.connect(&back, SIGNAL(regionSelected(QRect)), SLOT(quit()));
    return a.exec();
}

#include "main.moc"
