#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QGradient>
#include <QMouseEvent>
#include <QPropertyAnimation>

class Slider : public QWidget {
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE {
        QPainter p(this);
        QLinearGradient g(QPointF(0,0), QPointF(rect().bottomRight()));
        g.setColorAt(0, Qt::blue);
        g.setColorAt(1, Qt::gray);
        p.setBackground(g);
        p.eraseRect(rect());
        p.setPen(Qt::yellow);
        p.setFont(QFont("Helvetica", 48));
        p.drawText(rect(), "Click Me To Hide");
    }
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE {
        hide();
    }
public:
    explicit Slider(QWidget *parent = 0) : QWidget(parent) {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
};

class Window : public QWidget {
    QGridLayout m_layout;
    Slider m_slider;
    QLabel m_label;
    QPropertyAnimation m_animation;
public:
    explicit Window(QWidget *parent = 0, Qt::WindowFlags f = 0) :
        QWidget(parent, f),
        m_layout(this),
        m_slider(this),
        m_animation(&m_slider, "pos")
   {
        setMouseTracking(true);
        m_layout.addWidget(&m_label);
        m_slider.hide();
        m_slider.setMouseTracking(false);
        m_animation.setStartValue(QPoint(-width(), 0));
        m_animation.setEndValue(QPoint(0, 0));
        m_animation.setDuration(500);
        m_animation.setEasingCurve(QEasingCurve::InCubic);
    }
    void leaveEvent(QEvent *) {
        if (window() && QCursor::pos().x() <= window()->geometry().topLeft().x()) {
            showSlider();
        }
    }
    void childEvent(QChildEvent * ev) {
        if (ev->added() && ev->child()->isWidgetType()) {
            ev->child()->installEventFilter(this);
            static_cast<QWidget*>(ev->child())->setMouseTracking(true);
        }
    }
    bool event(QEvent * ev) {
        eventFilter(this, ev);
        return QWidget::event(ev);
    }
    bool eventFilter(QObject *, QEvent * ev) {
        if (ev->type() == QEvent::MouseMove) {
            auto pos = QCursor::pos();
            if (window() && window()->isFullScreen()) {
                if (pos.x() <= window()->geometry().topLeft().x()) {
                    showSlider();
                }
            }
            m_label.setText(QString("%1, %2").arg(pos.x()).arg(pos.y()));
        }
        return false;
    }
    void resizeEvent(QResizeEvent *) {
        m_slider.resize(size());
        m_animation.setStartValue(QPoint(-width(), 0));
    }
    Q_SLOT void showSlider() {
        if (m_slider.isVisible() || (window() && qApp->activeWindow() != window())) return;
        m_slider.raise();
        m_slider.show();
        m_animation.start();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}
