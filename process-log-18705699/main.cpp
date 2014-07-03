    #include <QWidget>
    #include <QPushButton>
    #include <QTextEdit>
    #include <QProcess>
    #include <QVBoxLayout>
    #include <QApplication>

    class Widget : public QWidget
    {
        Q_OBJECT
        QPushButton* addBtn;
        QTextEdit* text;
    public:
        Widget() {
            addBtn = new QPushButton("Add Module");
            text = new QTextEdit();
            text->setReadOnly(true);
            QVBoxLayout* layout = new QVBoxLayout(this);
            layout->addWidget(addBtn,0);
            layout->addWidget(text);
            connect(addBtn,SIGNAL(clicked()),SLOT(launchModule()));
        }
        Q_SLOT void launchModule() {
            QString program = "C:/A2Q2-build-desktop/debug/A2Q1.exe";
            QProcess *myProcess = new QProcess(this);
            connect(myProcess, SIGNAL(finished(int)), SLOT(finished()));
            connect(myProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(finished()));
            myProcess->start(program);
        }

        Q_SLOT void finished() {
            QProcess *process = qobject_cast<QProcess*>(sender());
            QString out = process->readAllStandardOutput(); // will be empty if the process failed to start
            text->append(out);
            process->deleteLater();
        }
    };

    int main(int argc, char **argv)
    {
        QApplication app(argc, argv);
        Widget w;
        w.show();
        app.exec();
    }

    #include "main.moc"
