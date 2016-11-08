// https://github.com/KubaO/stackoverflown/tree/master/questions/sqlite-blob-11062145
#include <QtSql>

int main()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./log.db");

    if (!db.open()) { qDebug() << "can't open the database"; return 1; }

    QSqlQuery query{db};

    query.exec("DROP TABLE log");

    if (!query.exec("CREATE TABLE log(packet BLOB)"))
        qDebug() << "create table failed";

    QVariant data[2] = {QByteArray{1024, 1}, QByteArray{2048, 2}};

    query.prepare("INSERT INTO log VALUES(:packet)");

    query.bindValue(":packet", data[0], QSql::In | QSql::Binary);
    if (!query.exec()) qDebug() << "insert failed";

    query.bindValue(":packet", data[1], QSql::In | QSql::Binary);
    if (!query.exec()) qDebug() << "insert failed";

    db.close();

    if (!db.open()) { qDebug() << "can't reopen the database"; return 2; }

    query.prepare("SELECT (packet) FROM log");
    if (!query.exec()) qDebug() << "select failed";

    for (auto const & d : data) if (query.next()) {
        qDebug() << query.value(0).toByteArray().size() << d.toByteArray().size();
        if (d != query.value(0)) qDebug() << "mismatched readback value";
    }

    db.close();
}
