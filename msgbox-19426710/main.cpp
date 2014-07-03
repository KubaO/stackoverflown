#include <QPushButton>
#include <QMessageBox>
#include <QApplication>

QString globalMessage;

class Failer : public QObject {
    Q_OBJECT
public:
    Q_SLOT void failure() {
        globalMessage = "Houston, we have got a problem.";
        qApp->exit(1);
    }
};

int main(int argc, char ** argv) {
    QApplication app(argc, argv);
    QPushButton pb("Fail Me");
    Failer failer;
    failer.connect(&pb, SIGNAL(clicked()), SLOT(failure()));
    pb.show();
    int rc = app.exec();
    if (rc) {
        QMessageBox::critical(NULL, "A problem has occurred...", globalMessage, QMessageBox::Ok);
    }
    return rc;
}

#include "main.moc"
