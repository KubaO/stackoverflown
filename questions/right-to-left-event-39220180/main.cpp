// https://github.com/KubaO/stackoverflown/tree/master/questions/right-to-left-event-39220180
#include <QtWidgets>

class RightToLeftClick : public QObject {
    bool eventFilter(QObject *watched, QEvent *event) {
        if (event->type() == QEvent::MouseButtonDblClick ||
                event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseButtonRelease) {
            auto ev = static_cast<QMouseEvent*>(event);
            if (ev->button() == Qt::RightButton) {
                auto buttons = ev->buttons();
                if (buttons & Qt::RightButton) {
                    buttons ^= Qt::RightButton;
                    buttons |= Qt::LeftButton;
                }
                QMouseEvent lev{ev->type(),
                            ev->localPos(),
                            ev->windowPos(),
                            ev->screenPos(),
                            Qt::LeftButton,
                            buttons,
                            ev->modifiers(),
                            ev->source()};
                Q_ASSERT(! (lev.buttons() & Qt::RightButton));
                QCoreApplication::sendEvent(watched, &lev);
                return true;
            }
        }
        return QObject::eventFilter(watched, event);
    }
public:
    explicit RightToLeftClick(QObject * parent = nullptr) : QObject{parent} {
        addTo(parent);
    }
    void addTo(QObject * obj) {
        if (obj) obj->installEventFilter(this);
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QWidget ui;
    QVBoxLayout layout{&ui};
    QPushButton button1{"Left Click Me"};
    QPushButton button2{"Right Click Me"};
    layout.addWidget(&button1);
    layout.addWidget(&button2);
    ui.show();
    RightToLeftClick rtl{&button2};
    return app.exec();
}
