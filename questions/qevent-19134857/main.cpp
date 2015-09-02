#include <QCoreApplication>
#include <QEvent>
#include <QString>
#include <QDebug>

#define CPP11

// A type-identifier-generating wrapper for events
template <typename Derived> class EventWrapper : public QEvent {
public:
    EventWrapper() : QEvent(staticType()) {}
    static QEvent::Type staticType() {
        static QEvent::Type type = static_cast<QEvent::Type>(registerEventType());
        return type;
    }
    static bool is(const QEvent * ev) { return ev->type() == staticType(); }
    static Derived* cast(QEvent * ev) { return is(ev) ? static_cast<Derived*>(ev) : 0; }
};

/* Code for the Question */

// The metafactory for string events
template <typename Derived> class QueStringEventMF {
    class Final;
    QueStringEventMF();
    QueStringEventMF(const QueStringEventMF &);
public:
    class Event : public EventWrapper<Derived>, private virtual Final {
        QString m_str;
    public:
        explicit Event(const QString & str) : m_str(str) {}
        QString value() const { return m_str; }
        static QString value(const QEvent * ev) {
            if (EventWrapper<Derived>::is(ev)) return static_cast<Event*>(ev)->value();
            return QString(); // null string
        }
    };
private:
    class Final {
        friend class Event;
    private:
        Final() {}
        Final(const Final &) {}
    };
};

class QueUpdate : public QueStringEventMF<QueUpdate> {};
class QueClear : public QueStringEventMF<QueClear> {};
void test1() {
    QueUpdate::Event * ev = new QueUpdate::Event("foo");
    Q_UNUSED(ev);
}

template <typename Derived> class StringEvent : public EventWrapper<Derived> {
    QString m_str;
public:
    explicit StringEvent(const QString & str) : m_str(str) {}
    QString value() const { return m_str; }
};

template <typename Derived> class QueStringEventMF2 {
public:
    typedef StringEvent<Derived> Event;
};

class QueUpdate2 : public QueStringEventMF2<QueUpdate2> {};
class QueClear2 : public QueStringEventMF2<QueClear2> {};

void test2() {
    QueUpdate2::Event * ev = new QueUpdate2::Event("foo");
    Q_UNUSED(ev);
}

#if defined(CPP11)

/* C++11 */

// The generic metafactory for unique event types that carry data
template <typename Derived, class Data> class EventMF {
    class Final;
    EventMF();
    EventMF(const EventMF &);
    ~EventMF();
public:
    class Event : public EventWrapper<Event>, public Data, private virtual Final {
    public:
        template<typename... Args>
        Event(Args&&... args): Data(std::forward<Args>(args)...) {}
    };
private:
    class Final {
        friend class Event;
    private:
        Final() {}
        Final(const Final &) {}
    };
};

// A string carrier
class StringData {
    QString m_str;
public:
    explicit StringData(const QString & str) : m_str(str) {}
    QString value() const { return m_str; }
};

#else

/* C++98 */

// The generic event metafactory
template <typename Derived, template <typename> class Carrier> class EventMF {
    class EventFwd;
    class Final;
    class FinalWrapper : public EventWrapper<EventFwd>, public virtual Final {};
public:
    // EventFwd is a class derived from Event. The EventWrapper's cast()
    // will cast to a covariant return type - the derived class. That's OK.
    typedef Carrier<FinalWrapper> Event;
private:
    class EventFwd : public Event {};
    class Final {
        friend class FinalWrapper;
        friend class Carrier<FinalWrapper>;
    private:
        Final() {}
        Final(const Final &) {}
    };
};

// A string carrier
template <typename Base> class StringData : public Base {
    QString m_str;
public:
    explicit StringData(const QString & str) : m_str(str) {}
    QString value() const { return m_str; }
};

#endif

// A string event metafactory
template <typename Derived> class StringEventMF : public EventMF<Derived, StringData> {};

class Update : public EventMF<Update, StringData> {}; // using generic metafactory
class Clear : public StringEventMF<Clear> {}; // using specific metafactory
#if 0
// This should fail at compile time as such derivation would produce classes with
// duplicate event types. That's what the Final class was for in the matafactory.
class Error : public Update::Event { Error() : Update::Event("") {} };
#endif

int main(int, char**)
{
    // Test that it works as expected.
    Update::Event update("update");
    Clear::Event clear("clear");
    Q_ASSERT(Update::Event::staticType() != Clear::Event::staticType());
    Q_ASSERT(Update::Event::staticType() == Update::Event::cast(&update)->staticType());
    qDebug() << Update::Event::cast(&update)->value();
    Q_ASSERT(Update::Event::cast(&clear) == 0);
    qDebug() << Clear::Event::cast(&clear)->value();
    Q_ASSERT(Clear::Event::cast(&update) == 0);
}
