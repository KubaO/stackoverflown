// From a wonderful answer by Leemes http://stackoverflow.com/a/32539990/1329652
#include "posixsignalproxy.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

static std::array<int,2> & makePair(std::array<int,2> & sockets) {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data()))
       qFatal("PosixSignalProxy: Couldn't create socket pair");
    return sockets;
}

PosixSignalProxy::PosixSignalProxy(int signum, int flags, QObject *parent, Sockets & sockets, void(*handler)(int)) :
    QObject(parent),
    m_signum(signum),
    m_sockets(makePair(sockets)),
    m_notifier(m_sockets[1], QSocketNotifier::Read, this)
{
    connect(&m_notifier, &QSocketNotifier::activated, this, &PosixSignalProxy::handleSignal);

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = flags;

    if (sigaction(m_signum, &sa, 0) > 0)
        qFatal("PosixSignalProxy: Couldn't register POSIX signal handler");
}

void PosixSignalProxy::staticSignalHandler(PosixSignalProxy::Sockets & sockets)
{
    char tmp = 1;
    ::write(sockets[0], &tmp, sizeof(tmp));
}

void PosixSignalProxy::handleSignal()
{
    m_notifier.setEnabled(false);
    char tmp;
    ::read(m_sockets[1], &tmp, sizeof(tmp));

    // Here, we're allowed to do Qt stuff.
    emit receivedSignal();

    m_notifier.setEnabled(true);
}
