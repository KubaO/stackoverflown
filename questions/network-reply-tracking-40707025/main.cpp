// https://github.com/KubaO/stackoverflown/tree/master/questions/network-reply-tracking-40707025
#include <QtNetwork>

class MyCtl : public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager{this};
    // ...
    void reply_finished(QNetworkReply *reply);
public:
    MyCtl(QObject *parent = nullptr);
    void do_log_in(const QString &email, const QString &password);
};

static const char kAuthGetSalt[] = "req_auth-get-salt";
static const char kDoAuth[] = "req_do-auth";
static const char kEmail[] = "req_email";
static const char kPassword[] = "req_password";

static const auto authGetSaltUrl = QStringLiteral("https://myapp.com/auth-get-salt.php?email=%1");
static const auto doAuthUrl = QStringLiteral("https://myapp.com/do-auth.php?email=%1&passwordHash=%2");

MyCtl::MyCtl(QObject *parent) : QObject{parent}
{
    connect(&manager, &QNetworkAccessManager::finished, this, &MyCtl::reply_finished);
}

void MyCtl::do_log_in(const QString &email, const QString &password) {
    auto url = authGetSaltUrl.arg(email);
    auto reply = manager.get(QNetworkRequest{url});
    reply->setProperty(kAuthGetSalt, true);
    reply->setProperty(kEmail, email);
    reply->setProperty(kPassword, password);
}

void MyCtl::reply_finished(QNetworkReply *reply) {
    if (!reply->property(kAuthGetSalt).isNull()) {
        reply->deleteLater(); // let's not leak the reply
        if (reply->error() == QNetworkReply::NoError) {
            auto salt = reply->readAll();
            auto email = reply->property(kEmail).toString();
            auto password = reply->property(kPassword).toString();
            Q_ASSERT(!password.isEmpty() && !email.isEmpty());
            QCryptographicHash hasher{QCryptographicHash::Sha1};
            hasher.addData(salt); // the server must hash the same way
            hasher.addData("----");
            hasher.addData(password.toUtf8());
            auto hash = hasher.result().toBase64(QByteArray::Base64UrlEncoding);
            auto url = doAuthUrl.arg(email).arg(QString::fromLatin1(hash));
            auto reply = manager.get(QNetworkRequest{url});
            reply->setProperty(kDoAuth, true);
            reply->setProperty(kEmail, email);
        }
    }
    else if (!reply->property(kDoAuth).isNull()) {
        if (reply->error() == QNetworkReply::NoError) {
            auto email = reply->property(kEmail).toString();
            // ...
        }
    }
}

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    MyCtl ctl;
    return app.exec();
}
#include "main.moc"
