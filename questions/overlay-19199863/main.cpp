// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-19199863
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Overlay : public QWidget {
public:
    Overlay(QWidget * parent = nullptr) : QWidget{parent} {
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter{this}.fillRect(rect(), {80, 80, 255, 128});
    }
};

class Filter : public QObject {
    QPointer<Overlay> m_overlay;
    QPointer<QWidget> m_overlayOn;
public:
    Filter(QObject * parent = nullptr) : QObject{parent} {}
protected:
    bool eventFilter(QObject * obj, QEvent * ev) override {
        if (!obj->isWidgetType()) return false;
        auto w = static_cast<QWidget*>(obj);
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
    QApplication a{argc, argv};
    Filter filter;
    QWidget window;
    QHBoxLayout layout{&window};
    for (auto text : { "Foo", "Bar", "Baz "}) {
        auto label = new QLabel{text};
        layout.addWidget(label);
        label->installEventFilter(&filter);
    }
    window.setMinimumSize(300, 250);
    window.show();
    return a.exec();
}
