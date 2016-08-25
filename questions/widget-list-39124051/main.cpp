// https://github.com/KubaO/stackoverflown/tree/master/questions/widget-list-39124051
#include <QtWidgets>
#include <private/qframe_p.h>
#include <array>
#include <list>
#include <vector>

struct H : QWidget {
    void * data;
    static void swap_d(QObject * a, QObject * b) { static_cast<H*>(a)->d_ptr.swap(static_cast<H*>(b)->d_ptr); }
    static void swap_data(QWidget * a, QWidget *b) {
        std::swap((&static_cast<H*>(a)->data)[-1], (&static_cast<H*>(b)->data)[-1]);
    }
    static QObjectPrivate * d(QObject * o) { return static_cast<QObjectPrivate*>(static_cast<H*>(o)->d_ptr.data()); }
    static QWidgetPrivate * dw(QWidget * o) { return static_cast<QWidgetPrivate*>(static_cast<H*>(o)->d_ptr.data()); }
};

struct LH : QWidgetItem {
    static void setWidget(QLayoutItem * item, QWidget * w) {
        if (item->widget())
            static_cast<LH*>(item)->wid = w;
    }
};

struct PH : QPaintDevice {
    QPaintDevicePrivate * reserved2;
    static void swap_paintDevice(QWidget * a, QWidget * b) {
        auto pa = static_cast<PH*>(static_cast<QPaintDevice*>(a));
        auto pb = static_cast<PH*>(static_cast<QPaintDevice*>(b));
        std::swap(pa->painters, pb->painters);
        std::swap((&pa->reserved2)[-1], (&pb->reserved2)[-1]); // ugly hack
    }
};

QObject * setParent(QObject * obj, QObject * parent) {
    auto prevParent = obj->parent();
    if (obj->isWidgetType())
        static_cast<QWidget*>(obj)->setParent(qobject_cast<QWidget*>(parent));
    else
        obj->setParent(parent);
    return prevParent;
}

void setQPtr(QObjectPrivate * p, QObject * old) {
    for (auto child : p->children)
        H::d(child)->parent = p->q_ptr;
    if (! p->parent) return;
    auto pp = H::d(p->parent);
    for (auto & child : pp->children) {
        if (child != old) continue;
        child = p->q_ptr;
        return;
    }
    Q_ASSERT(false);
}

void setLayoutItem(QWidget * w, QWidget * newWidget) {
    auto parentLayout = w->parentWidget() ? w->parentWidget()->layout() : nullptr;
    if (!parentLayout) return;
    for (int i = 0; i < parentLayout->count(); ++i) {
        auto item = parentLayout->itemAt(i);
        if (item->widget() != w) continue;
        LH::setWidget(item, newWidget);
        return;
    }
    Q_ASSERT(false);
}

void swapWidgets(QWidget * aw, QWidget * bw) {
    qDebug() << " swapping" << aw << "(" << aw->parent() << ")" << bw << "(" << bw->parent() << ")";
    QCoreApplication::sendPostedEvents(aw);
    QCoreApplication::sendPostedEvents(bw);
    QSignalBlocker aBlock(aw);
    QSignalBlocker bBlock(bw);

    setLayoutItem(aw, bw);
    setLayoutItem(bw, aw);

    auto a = H::dw(aw), b = H::dw(bw);
    std::swap(a->q_ptr, b->q_ptr);
    setQPtr(a, b->q_ptr);
    setQPtr(b, a->q_ptr);

    std::swap(a->focus_next, b->focus_next);
    std::swap(a->focus_prev, b->focus_prev);
    H::swap_d(aw, bw);
    H::swap_data(aw, bw);
    PH::swap_paintDevice(aw, bw);
}

class AppointmentSchedulePrivate {
public:
    QVBoxLayout layout;
    QDateTimeEdit start;
    QDateTimeEdit end;
};

class AppointmentSchedule : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), AppointmentSchedule)
    QScopedPointer<AppointmentSchedulePrivate> dd_ptr { new AppointmentSchedulePrivate };
public:
    AppointmentSchedule(AppointmentSchedule && other) :
        dd_ptr{other.dd_ptr.take()}
    {
        qDebug() << "  moving from" << &other << "to" << this;
        swapWidgets(this, &other);

        Q_D(AppointmentSchedule);
        qDebug() << this << parent() << layout() << d->layout.parentWidget();
        Q_ASSERT(d->layout.parentWidget() == this);
        Q_ASSERT(d->start.parentWidget() == this);
        Q_ASSERT(d->end.parentWidget() == this);
    }
    explicit AppointmentSchedule(QWidget *parent = 0) :
        QFrame{parent}
    {
        qDebug() << "  constructing" << this;
        Q_D(AppointmentSchedule);
        setLayout(&d->layout);
        d->layout.addWidget(&d->start);
        d->layout.addWidget(&d->end);
        d->layout.setMargin(2);
        d->layout.setSpacing(2);
        setFrameStyle(QFrame::Panel);
    }
    void setStart(const QDateTime & val) {
        Q_D(AppointmentSchedule);
        d->start.setDateTime(val);
    }
    void setEnd(const QDateTime & val) {
        Q_D(AppointmentSchedule);
        d->end.setDateTime(val);
    }
};



int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QWidget w;
    QVBoxLayout layout{&w};
    std::array<QDate, 2> dates { QDate{2000, 1, 1}, QDate{2016,1,1} };
    qDebug() << sizeof(QObject)-sizeof(void*) << sizeof(QPaintDevice)-sizeof(void*) << sizeof(QWidget)-sizeof(void*);

    std::list<AppointmentSchedule> schedules1;
    for (auto & date : dates) {
        schedules1.emplace_back();
        auto & schedule = schedules1.back();
        schedule.setStart({date, QTime{8, 0, 0}}); // uniform initialization!
        schedule.setEnd({date, QTime{8, 15, 0}});
        layout.addWidget(&schedule);
    }

    qDebug() << "  about to make a vector";
    std::vector<AppointmentSchedule> schedules2;
    for (auto & date : dates) {
        qDebug() << "  moving to a vector";
        schedules2.push_back(AppointmentSchedule{});
        //schedules1.emplace_back();
        auto & schedule = schedules2.back();
        schedule.setStart({date, QTime{8, 0, 0}}); // uniform initialization!
        schedule.setEnd({date, QTime{8, 15, 0}});
        layout.addWidget(&schedule);
    }

    w.show();
    return app.exec();
}
#include "main.moc"
