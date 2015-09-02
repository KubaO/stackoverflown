#include <QApplication>
#include <QMainWindow>
#include <QResizeEvent>
#include <QPainter>

class OverlayWidget : public QWidget
{
public:
    explicit OverlayWidget(QWidget * parent = 0) : QWidget(parent) {
        if (parent) {
            parent->installEventFilter(this);
            raise();
        }
    }
protected:
    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject * obj, QEvent * ev) {
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
    bool event(QEvent* ev) {
        if (ev->type() == QEvent::ParentAboutToChange) {
            if (parent()) parent()->removeEventFilter(this);
        }
        else if (ev->type() == QEvent::ParentChange) {
            if (parent()) {
                parent()->installEventFilter(this);
                raise();
            }
        }
        return QWidget::event(ev);
    }
};

class LoadingOverlay : public OverlayWidget
{
public:
    LoadingOverlay(QWidget * parent = 0) : OverlayWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        p.fillRect(rect(), QColor(100, 100, 100, 128));
        p.setPen(QColor(200, 200, 255, 255));
        p.setFont(QFont("arial,helvetica", 48));
        p.drawText(rect(), "Loading...", Qt::AlignHCenter | Qt::AlignVCenter);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;
    new LoadingOverlay(&w);
    w.show();
    return a.exec();
}
