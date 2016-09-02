// https://github.com/KubaO/stackoverflown/tree/master/questions/net-request-seq-39285844
#include <QtWidgets>
#include <QtNetwork>
#include <list>

class Sequence : QObject {
    Q_OBJECT
public:
    Sequence() {}
    template <typename Object, typename S>
    Sequence & add(std::function<Object*()> && fun, S signal) {
        m_steps.push_back({std::move(fun),
                           [signal](QObject* obj) {
                               QObject::connect(static_cast<Object*>(obj), signal, []{});
                           }});

        return *this;
    }
    void run() {
        if (m_steps.empty()) return;
        auto obj = m_steps.front().request();
        m_steps.front().connect(obj);
    }
    void run(std::list<Step>::const_iterator step) {

    }

private:
    struct Step {
        std::function<QObject*()> request;
        std::function<void(QObject*)> connect;
        std::list<Step>::const_iterator next;
    };
    std::list<Step> m_steps;
};

Sequence & operator<<(Sequence & seq, std::function<QNetworkReply*()> && step) {
    return seq.add(std::move(step), &QNetworkReply::finished);
}

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QNetworkAccessManager mgr;
    Sequence seq;

    seq << [&]{ return mgr.get(QNetworkRequest{{"Foo"}}); };

    //return app.exec();
}

#include "main.moc"
