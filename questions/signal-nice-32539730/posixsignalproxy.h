// From a wonderful answer by Leemes http://stackoverflow.com/a/32539990/1329652
#ifndef POSIXSIGNALPROXY_H
#define POSIXSIGNALPROXY_H

#include <QSocketNotifier>
#include <array>

template <int N> struct PosixSignal {};

class PosixSignalProxy : public QObject
{
    Q_OBJECT
    typedef std::array<int,2> Sockets;
    PosixSignalProxy(int signum, int flags, QObject * parent, Sockets &, void (*handler)(int));
public:
    template <int N>
    PosixSignalProxy(PosixSignal<N>, int posixSignalFlags, QObject *parent = nullptr) :
        PosixSignalProxy(N, posixSignalFlags, parent, Data<N>::sockets(), Data<N>::handler) {}
    template <int N>
    PosixSignalProxy(PosixSignal<N>, QObject *parent = nullptr) :
        PosixSignalProxy(PosixSignal<N>(), 0, parent) {}

    /**
     * Qt signal which is emitted right after receiving and handling the POSIX
     * signal. In the Qt signal handler (slot) you are allowed to do anything.
     */
    Q_SIGNAL void receivedSignal();
    /// The POSIX signal number
    int signal() const { return m_signum; }
private:
    template <int N> struct Data {
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
