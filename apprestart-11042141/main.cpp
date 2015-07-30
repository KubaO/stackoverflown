#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QDir>
#include <QProcess>

class Starter : public QObject {
    Q_OBJECT
public:
    Q_SLOT void start() {
        QString app = QApplication::applicationFilePath();
        QStringList arguments = QApplication::arguments();
        QString wd = QDir::currentPath();
        qDebug() << app << arguments << wd;
        QProcess::startDetached(app, arguments, wd);
        QApplication::exit();
    }
};

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    QPushButton button("Spawn");
    Starter starter;
    starter.connect(&button, SIGNAL(clicked()), SLOT(start()));
    button.show();
    return app.exec();
}

#include "main.moc"
