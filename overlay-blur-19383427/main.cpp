#include <QApplication>
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>
#include <QGraphicsBlurEffect>

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE override
#endif

class OverlayWidget : public QWidget
{
    void newParent() {
        if (!parent()) return;
        parent()->installEventFilter(this);
        raise();
    }
public:
    explicit OverlayWidget(QWidget * parent = 0) : QWidget(parent) {
        setAttribute(Qt::WA_NoSystemBackground);
        newParent();
    }
protected:
    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject * obj, QEvent * ev) Q_DECL_OVERRIDE {
        if (obj == parent()) {
            if (ev->type() == QEvent::Resize) {
                QResizeEvent * rev = static_cast<QResizeEvent*>(ev);
                this->resize(rev->size());
            }
            else if (ev->type() == QEvent::ChildAdded) {
                raise();
            }
        }
        return QWidget::eventFilter(obj, ev);
    }
    //! Tracks parent widget changes
    bool event(QEvent* ev) Q_DECL_OVERRIDE {
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
    explicit ContainerWidget(QWidget * parent = 0) : QWidget(parent) {}
    inline void setSize(QObject * obj) {
        if (obj->isWidgetType()) static_cast<QWidget*>(obj)->setGeometry(rect());
    }
protected:
    //! Resizes children to fill the extent of this widget
    bool event(QEvent * ev) Q_DECL_OVERRIDE {
        if (ev->type() == QEvent::ChildAdded) {
            setSize(static_cast<QChildEvent*>(ev)->child());
        }
        return QWidget::event(ev);
    }
    //! Keeps the children appropriately sized
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE {
        foreach(QObject * obj, children()) setSize(obj);
    }
};

class LoadingOverlay : public OverlayWidget
{
public:
    LoadingOverlay(QWidget * parent = 0) : OverlayWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE {
        QPainter p(this);
        p.fillRect(rect(), QColor(100, 100, 100, 128));
        p.setPen(QColor(200, 200, 255, 255));
        p.setFont(QFont("arial,helvetica", 48));
        p.drawText(rect(), "Loading...", Qt::AlignHCenter | Qt::AlignTop);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ContainerWidget base;
    QLabel l("Dewey, Cheatem and Howe, LLC.", &base);
    l.setFont(QFont("times,times new roman", 32));
    l.setAlignment(Qt::AlignCenter);
    l.setGraphicsEffect(new QGraphicsBlurEffect);
    LoadingOverlay overlay(&base);
    base.show();
    return a.exec();
}
