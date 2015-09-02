    //main.cpp
    #include <QApplication>
    #include <QPushButton>
    #include <QVBoxLayout>
    #include <QLabel>
    #include <QElapsedTimer>
    #include <QTime>

    class Window : public QWidget {
        Q_OBJECT
        int m_timerId;
        qint64 m_accumulator;
        QLabel *m_label;
        QElapsedTimer m_timer;
        Q_SLOT void on_restart_clicked() {
            m_accumulator = 0;
            m_timer.restart();
            if (m_timerId == -1) m_timerId = startTimer(50);
        }
        Q_SLOT void on_pause_clicked() {
            if (m_timer.isValid()) {
                m_accumulator += m_timer.elapsed();
                m_timer.invalidate();
            } else {
                m_timer.restart();
                m_timerId = startTimer(50);
            }
        }
        void timerEvent(QTimerEvent * ev) {
            if (ev->timerId() != m_timerId) {
                QWidget::timerEvent(ev);
                return;
            }
            QTime t(0,0);
            t = t.addMSecs(m_accumulator);
            if (m_timer.isValid()) {
                t = t.addMSecs(m_timer.elapsed());
            } else {
                killTimer(m_timerId);
                m_timerId = -1;
            }
            m_label->setText(t.toString("h:m:ss.zzz"));
        }
    public:
        explicit Window(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f), m_timerId(-1)  {
            QVBoxLayout * l = new QVBoxLayout(this);
            QPushButton * restart = new QPushButton("Start");
            QPushButton * pause = new QPushButton("Pause/Resume");
            restart->setObjectName("restart");
            pause->setObjectName("pause");
            m_label = new QLabel("--");
            l->addWidget(restart);
            l->addWidget(pause);
            l->addWidget(m_label);
            QMetaObject::connectSlotsByName(this);
        }
    };

    int main(int argc, char *argv[])
    {
        QApplication a(argc, argv);
        Window w;
        w.show();
        return a.exec();
    }

    #include "main.moc"
