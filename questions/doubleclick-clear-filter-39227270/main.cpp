// https://github.com/KubaO/stackoverflown/tree/master/questions/doubleclick-clear-filter-39227270
#include <QtWidgets>

class ClearOnDoubleClick : public QObject {
    bool eventFilter(QObject *watched, QEvent *event) {
        if (event->type() == QEvent::MouseButtonDblClick)
            QMetaObject::invokeMethod(watched, "clear");
        return QObject::eventFilter(watched, event);
    }
public:
    explicit ClearOnDoubleClick(QObject * parent = nullptr) : QObject{parent} {
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
    QLineEdit edit;
    layout.addWidget(&edit);
    ui.show();
    ClearOnDoubleClick clear{&edit};
    return app.exec();
}
