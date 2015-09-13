// From a wonderful answer by Leemes http://stackoverflow.com/a/32539990/1329652
#ifndef POSIXSIGNALPROXY_H
#define POSIXSIGNALPROXY_H

#include <QSocketNotifier>
#include <array>

class PosixSignalProxy : public QObject
{
    Q_OBJECT
    typedef std::array<int,2> Sockets;
    PosixSignalProxy(int signum, int flags, QObject * parent, Sockets &, void (*handler)(int));
public:
    template <typename T>
    PosixSignalProxy(T, int signum, int posixSignalFlags, QObject *parent = nullptr) :
        PosixSignalProxy(signum, posixSignalFlags, parent, Data<T>::sockets(), Data<T>::handler) {}
    template <typename T>
    PosixSignalProxy(T p, int signum, QObject *parent = nullptr) :
        PosixSignalProxy(p, signum, 0, parent) {}

    /**
     * Qt signal which is emitted right after receiving and handling the POSIX
     * signal. In the Qt signal handler (slot) you are allowed to do anything.
     */
    Q_SIGNAL void receivedSignal();
    /// The POSIX signal number
    int signal() const { return m_signum; }
private:
    template <typename T> struct Data {
        static Sockets & sockets() {
            static Sockets data;
            return data;
        }
        static void handler(int) {
            PosixSignalProxy::staticSignalHandler(sockets());
        }
    };

    int m_signum;
    Sockets & m_sockets;
    QSocketNotifier m_notifier;

    void handleSignal();
    static void staticSignalHandler(Sockets &);
};

#endif // POSIXSIGNALPROXY_H
