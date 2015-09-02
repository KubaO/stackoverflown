#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPointer>

class Overlay : public QWidget {
public:
    Overlay(QWidget * parent = 0) : QWidget(parent) {
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        p.fillRect(rect(), QColor(80, 80, 255, 128));
    }
};

class Filter : public QObject {
    QPointer<Overlay> m_overlay;
    QPointer<QWidget> m_overlayOn;
public:
    Filter(QObject * parent = 0) : QObject(parent) {}
protected:
    bool eventFilter(QObject * obj, QEvent * ev) {
        if (!obj->isWidgetType()) return false;
        QWidget * w = static_cast<QWidget*>(obj);
        if (ev->type() == QEvent::MouseButtonPress) {
            if (!m_overlay) m_overlay = new Overlay(w->parentWidget());
            m_overlay->setGeometry(w->geometry());
            m_overlayOn = w;
            m_overlay->show();
        }
        else if (ev->type() == QEvent::Resize) {
            if (m_overlay && m_overlayOn == w)
                m_overlay->setGeometry(w->geometry());
        }
        return false;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Filter filter;
    QWidget widget;
    QHBoxLayout * layout = new QHBoxLayout(&widget);
    layout->addWidget(new QLabel("Foo"));
    layout->addWidget(new QLabel("Bar"));
    layout->addWidget(new QLabel("Baz"));
    foreach (QObject * obj, widget.children()) {
        if (obj->isWidgetType()) obj->installEventFilter(&filter);
    }
    widget.show();
    return a.exec();
}
