#include <QApplication>
#include <QMessageBox>
#include <QBuffer>
#include <QScopedPointer>

void success()
{
    QScopedPointer<QBuffer> mysocket(new QBuffer);
    QMessageBox myBox;
    myBox.setInformativeText("Sucess.");
    myBox.setStandardButtons(QMessageBox::Ok);
    myBox.exec();

    mysocket->open(QBuffer::ReadWrite);
    mysocket->write(QByteArray(20, 0));
    mysocket->seek(0);

    qint16 blockSize = 0;
    QDataStream in(mysocket.data());
    in.setVersion(13);

    if (blockSize == 0) {
        if (mysocket->bytesAvailable() < (int)sizeof(quint16))
        {
            QMessageBox box;
            box.setInformativeText("return 1.");
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            return;
        }
        in >> blockSize;
    }

    if (mysocket->bytesAvailable() < blockSize)
    {
        QMessageBox box;
        box.setInformativeText("return 2");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QString result;
    in >> result;

    if ( result == "G" )
    {
        QMessageBox box;
        box.setInformativeText("Password Verified.");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
    else if (result == "N")
    {
        QMessageBox box;
        box.setInformativeText("Password Incorrect.");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
    else
    {
        QMessageBox box;
        box.setInformativeText("Error.");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    success();
    return 0;
}
