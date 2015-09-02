    // main.cpp
    #include <QCoreApplication>
    #include <QObjectList>
    #include <QList>
    #include <QThread>
    #include <iostream>

    const int cycleCount = 500; //!< number of cycles to run at, set to 0 to run forever
    const int threadCount = 5; //!< number of threads to create

    class Object : public QObject
    {
        Q_OBJECT
        int m_threadNumber;
        int m_delayUs;
        volatile bool m_active;
    public:
        explicit Object(int delayUs, int no) : m_threadNumber(no), m_delayUs(delayUs) {}
        Q_SIGNAL void indexReady(int);
        Q_SLOT void stop() { m_active = false; }
        Q_SLOT void start()
        {
            m_active = true;
            while (m_active) {
                usleep(m_delayUs);
                emit indexReady(m_threadNumber);
            }
        }
    };

    class Consumer : public QObject
    {
        Q_OBJECT
        QList<Object*> m_objects;
        QList<QThread*> m_threads;
        int m_threadCount; //!< number of active threads in m_threads
        int m_count;
    public:
        Consumer() : m_count(0) {}
        void addObject(Object * o) { m_objects << o; }
        void addThread(QThread * t) {
            m_threads << t;
            m_threadCount ++;
            connect(t, SIGNAL(finished()), SLOT(done()));
            connect(t, SIGNAL(terminated()), SLOT(done()));
        }
        Q_SLOT void ready(int n) {
            std::cout << "<" << m_count++ << ":" << n << ">" << std::endl;
            if (m_count == cycleCount) {
                foreach (Object * o, m_objects) o->stop();
                foreach (QThread * t, m_threads) t->wait();
            }
        }
        Q_SLOT void done() {
            QThread * t = qobject_cast<QThread*>(sender());
            int i = m_threads.indexOf(t);
            if (t) t->deleteLater();
            if (i>=0) {
                std::cout << "Thread " << i << " is done." << std::endl;
                m_threadCount --;
            }
            if (! m_threadCount) qApp->quit();
        }
    };

    int main(int argc, char *argv[])
    {
        Consumer c;
        QObjectList l;
        QCoreApplication a(argc, argv);
        std::cout << "Running under Qt version " << qVersion() << std::endl;
        for (int i = 0; i < threadCount; ++i) {
            Object * o = new Object(10000 + 5000*i, i+1);
            QThread * t = new QThread;
            c.addObject(o);
            c.addThread(t);
            o->moveToThread(t);
            o->connect(t, SIGNAL(started()), SLOT(start()));
            c.connect(o, SIGNAL(indexReady(int)), SLOT(ready(int)));
            t->start();
            t->exit();
            l << o;
        }
        return a.exec();
    }

    #include "main.moc"
