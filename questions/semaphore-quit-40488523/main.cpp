// https://github.com/KubaO/stackoverflown/tree/master/questions/semaphore-quit-40488523
#include <QtCore>

class Worker : public QRunnable {
    static const QString m_key;
    static QAtomicInteger<bool> m_abort;
    void run() override {
        QSystemSemaphore sem{m_key};
        qDebug() << "working";
        while (true) {
            sem.acquire();
            if (m_abort.load()) {
                sem.release();
                qDebug() << "aborting";
                return;
            }
            sem.release();
        }
    }
public:
    static void acquire(int n = 1) {
        QSystemSemaphore sem{m_key};
        while (n--)
            sem.acquire();
    }
    static void release(int n = 1) {
        QSystemSemaphore{m_key}.release(n);
    }
    static void quit(int n) {
        m_abort.store(true);
        release(n);
    }
};
const QString Worker::m_key = QStringLiteral("semaphore-quit-40488523");
QAtomicInteger<bool> Worker::m_abort = false;

int main()
{
    QThreadPool pool;
    QVarLengthArray<Worker, 20> workers{20};
    for (auto & worker : workers) {
        worker.setAutoDelete(false);
        pool.start(&worker);
    }
    Worker::release(workers.size()); // get some workers churning
    QThread::sleep(5);
    Worker::acquire(workers.size()); // pause all the workers
    Worker::quit(workers.size());
    // The thread pool destructor will wait for all workers to quit.
}
