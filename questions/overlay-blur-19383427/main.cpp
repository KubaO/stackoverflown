// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-widget-19362455
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class OverlayWidget : public QWidget
{
    void newParent() {
        if (!parent()) return;
        parent()->installEventFilter(this);
        raise();
    }
public:
    explicit OverlayWidget(QWidget * parent = nullptr) : QWidget{parent} {
        setAttribute(Qt::WA_NoSystemBackground);
        newParent();
    }
protected:
    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject * obj, QEvent * ev) override {
        if (obj == parent()) {
            if (ev->type() == QEvent::Resize)
                resize(static_cast<QResizeEvent*>(ev)->size());
            else if (ev->type() == QEvent::ChildAdded)
                raise();
        }
        return QWidget::eventFilter(obj, ev);
    }
    //! Tracks parent widget changes
    bool event(QEvent* ev) override {
        if (ev->type() == QEvent::ParentAboutToChange) {
            if (parent()) parent()->removeEventFilter(this);
        }
        else if (ev->type() == QEvent::ParentChange) {
            newParent();
        }
        return QWidget::event(ev);
    }
};

class ContainerWidget : public QWidget
{
public:
    explicit ContainerWidget(QWidget * parent = nullptr) : QWidget{parent} {}
    void setSize(QObject * obj) {
        if (obj->isWidgetType()) static_cast<QWidget*>(obj)->setGeometry(rect());
    }
protected:
    //! Resizes children to fill the extent of this widget
    bool event(QEvent * ev) override {
        if (ev->type() == QEvent::ChildAdded) {
            setSize(static_cast<QChildEvent*>(ev)->child());
        }
        return QWidget::event(ev);
    }
    //! Keeps the children appropriately sized
    void resizeEvent(QResizeEvent *) override {
        for(auto obj : children()) setSize(obj);
    }
};

class LoadingOverlay : public OverlayWidget
{
public:
    LoadingOverlay(QWidget * parent = nullptr) : OverlayWidget{parent} {
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p{this};
        p.fillRect(rect(), {100, 100, 100, 128});
        p.setPen({200, 200, 255});
        p.setFont({"arial,helvetica", 48});
        p.drawText(rect(), "Loading...", Qt::AlignHCenter | Qt::AlignTop);
    }
};

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
template <typename F> void singleShot(int period, F && fun) {
    struct Helper : public QObject {
        F fun;
        QBasicTimer timer;
        void timerEvent(QTimerEvent * event) override {
            if (event->timerId() != timer.timerId()) return;
            fun();
            deleteLater();
        }
        Helper(int period, F && fun) : fun(std::forward<F>(fun)) {
            timer.start(period, this);
        }
    };
    new Helper(period, std::forward<F>(fun));
}
#else
template <typename... Args>
auto singleShot(Args&&... args) -> decltype(QTimer::singleShot(std::forward<Args>(args)...)) {
    return QTimer::singleShot(std::forward<Args>(args)...);
}
#endif

int main(int argc, char *argv[])
{
    QApplication a{argc, argv};
    ContainerWidget base;
    QLabel label("Dewey, Cheatem and Howe, LLC.", &base);
    label.setFont({"times,times new roman", 32});
    label.setAlignment(Qt::AlignCenter);
    label.setGraphicsEffect(new QGraphicsBlurEffect);
    LoadingOverlay overlay(&base);
    base.show();
    singleShot(2000, [&]{
        overlay.hide();
        label.setGraphicsEffect(nullptr);
    });
    return a.exec();
}
