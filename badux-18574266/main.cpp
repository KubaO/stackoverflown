    #include <QApplication>
    #include <QWidget>
    #include <QGridLayout>
    #include <QLabel>
    #include <QPainter>
    #include <QGradient>
    #include <QMouseEvent>
    #include <QPropertyAnimation>

    class Slider : public QWidget {
    public:
        explicit Slider(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {
            setAttribute(Qt::WA_OpaquePaintEvent);
        }

    protected:
        void paintEvent(QPaintEvent *) {
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
        void mousePressEvent(QMouseEvent *) {
            hide();
        }
    };

    class Window : public QWidget {
        QWidget * m_slider;
        QLabel * m_label;
        QPropertyAnimation * m_animation;
    public:
        explicit Window(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {
            setMouseTracking(true);
            auto * l = new QGridLayout(this);
            m_slider = new Slider(this);
            m_label = new QLabel(this);
            l->addWidget(m_label);
            m_slider->hide();
            m_slider->setMouseTracking(false);
            m_animation = new QPropertyAnimation(m_slider, "pos", this);
            m_animation->setStartValue(QPoint(-width(), 0));
            m_animation->setEndValue(QPoint(0, 0));
            m_animation->setDuration(500);
            m_animation->setEasingCurve(QEasingCurve::InCubic);
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
                const QPoint pos = QCursor::pos();
                if (window() && window()->isFullScreen()) {
                    if (pos.x() <= window()->geometry().topLeft().x()) {
                        showSlider();
                    }
                }
                m_label->setText(QString("%1, %2").arg(pos.x()).arg(pos.y()));
            }
            return false;
        }
        void resizeEvent(QResizeEvent *) {
            m_slider->resize(size());
            m_animation->setStartValue(QPoint(-width(), 0));
        }
        Q_SLOT void showSlider() {
            if (m_slider->isVisible() || (window() && qApp->activeWindow() != window())) return;
            m_slider->raise();
            m_slider->show();
            m_slider->move(-width(), 0); // this may be redundant
            m_animation->start();
        }
    };

    int main(int argc, char *argv[])
    {
        QApplication a(argc, argv);
        Window w;
        w.show();
        return a.exec();
    }
