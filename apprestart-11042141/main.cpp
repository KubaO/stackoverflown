//main.cpp
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QProcess>

class Starter : public QObject {
    Q_OBJECT
public:
    Starter() {}
public slots:
    void start() {
        QString app = QApplication::applicationFilePath();
        QStringList arguments = QApplication::arguments();
        QString wd = QDir::currentPath();
        qDebug() << app << arguments << wd;
        QProcess::startDetached(app, arguments, wd);
        QApplication::exit();
    }
};

int main(int c, char ** v)
{
    QApplication a(c,v);
    QPushButton b("Spawn");
    Starter s;
    b.show();
    s.connect(&b, SIGNAL(clicked()), SLOT(start()));
    a.exec();
}

#include "main.moc"
