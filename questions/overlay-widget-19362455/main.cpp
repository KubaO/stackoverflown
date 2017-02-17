// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-widget-19362455
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class OverlayWidget : public QWidget
{
public:
    explicit OverlayWidget(QWidget * parent = nullptr) : QWidget{parent} {
        if (parent) {
            parent->installEventFilter(this);
            raise();
        }
    }
protected:
    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject * obj, QEvent * ev) override {
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
    bool event(QEvent* ev) override {
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
    LoadingOverlay(QWidget * parent = nullptr) : OverlayWidget{parent} {
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p{this};
        p.fillRect(rect(), {100, 100, 100, 128});
        p.setPen({200, 200, 255});
        p.setFont({"arial,helvetica", 48});
        p.drawText(rect(), "Loading...", Qt::AlignHCenter | Qt::AlignVCenter);
    }
};

int main(int argc, char * argv[])
{
    QApplication a{argc, argv};
    QMainWindow window;
    QLabel central{"Hello"};
    central.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    central.setMinimumSize(400, 300);
    LoadingOverlay overlay{&central};
    window.setCentralWidget(&central);
    window.show();
    return a.exec();
}
