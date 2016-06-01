// https://github.com/KubaO/stackoverflown/tree/master/questions/widget-animation-18531776
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
#include <QtWidgets>
typedef QOpenGLWidget GLWidget;
#elif QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
typedef QGLWidget GLWidget;
#else // Qt 4
#include <QtGui>
#include <QtOpenGL>
typedef QGLWidget GLWidget;
#endif

class Widget: public GLWidget
{
    QElapsedTimer m_timer;
    struct Animation : public QVariantAnimation {
       void updateCurrentValue(const QVariant &) {}
    } m_anim;
    QPolygonF m_polygon;
    qreal m_fps;
    void paintEvent(QPaintEvent *) {
        const qreal t = 0.05;
        qreal iFps = 1E9/m_timer.nsecsElapsed();
        m_fps = (1.0-t)*m_fps + t*iFps;
        int len = qMin(height(), width());
        QPainter p(this);
        p.drawText(rect(), QString("%1,%2 FPS").arg(m_fps, 0, 'f', 0).arg(iFps, 0, 'f', 0));
        p.translate(width()/2.0, height()/2.0);
        p.scale(len*.8, len*.8);
        p.rotate(m_anim.currentValue().toReal());
        p.setPen(QPen(Qt::darkBlue, 0.1));
        p.drawPolygon(m_polygon);
        p.end();
        m_timer.restart();
    }
public:
    Widget(QWidget *parent = 0) : GLWidget(parent), m_fps(0.0) {
        m_anim.setDuration(2000);
        m_anim.setStartValue(0);
        m_anim.setEndValue(360);
        m_anim.setEasingCurve(QEasingCurve::InBounce);
        m_anim.setLoopCount(-1);
        m_anim.start();
        m_polygon.resize(4);
        m_polygon[0] = QPointF(-0.3,  0);
        m_polygon[1] = QPointF(-0.5,  0.3);
        m_polygon[2] = QPointF( 0.5,  0);
        m_polygon[3] = QPointF(-0.5, -0.3);
        setAutoFillBackground(true);
        connect(&m_anim, SIGNAL(valueChanged(QVariant)), SLOT(update()));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
