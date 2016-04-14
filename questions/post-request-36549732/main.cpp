// https://github.com/KubaO/stackoverflown/tree/master/questions/post-request-36549732
// main.cpp
#include <QtNetwork>

int main(int argc, char ** argv)
{
    QCoreApplication a{argc, argv};
    QNetworkAccessManager manager;
    QByteArray post{"a="};
    post.append(QByteArray{512, 'b'});
    QNetworkRequest req(QUrl("http://server/test.php"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Must be a queued connection, or else the multi-threaded manager might
    // win the race and signal quit before `a.exec()` starts running. In such
    // case, the `quit()` is a NOP. We don't want that.
    QObject::connect(&manager, &QNetworkAccessManager::finished, &a, [](QNetworkReply * reply){
       qDebug() << reply->errorString();
       qApp->quit();
    }, Qt::QueuedConnection);

    manager.post(req, post);
    return a.exec();
}
