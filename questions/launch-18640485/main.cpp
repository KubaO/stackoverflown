    #include <QProcess>
    #include <QPushButton>
    #include <QApplication>

    class Window : public QPushButton {
        Q_OBJECT
        Q_SLOT void launch() {
            const QString program = "C:\\A2Q1-build-desktop\\debug\\A2Q1.exe";
            QProcess *process = new QProcess(this);
            connect(process, SIGNAL(finished(int)), SLOT(finished()));
            connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(finished()));
            process->start(program);
        }
        Q_SLOT void finished() {
            QProcess *process = qobject_cast<QProcess*>(sender());
            QString out = process->readAllStandardOutput(); // will be empty if the process failed to start
            delete process;
        }
    public:
        Window(QWidget *parent = 0) : QPushButton(parent) {
            connect(this, SIGNAL(clicked()), SLOT(launch()));
        }
    };

    int main (int argc, char **argv)
    {
        QApplication app(argc, argv);
        Window w;
        w.show();
        return app.exec();
    }

    #include "main.moc"
