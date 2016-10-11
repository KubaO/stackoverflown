// https://github.com/KubaO/stackoverflown/tree/master/questions/json-map-iter-39979440
#include <QtCore>

QVariantMap replaceMap(QVariantMap dst, const QVariantMap & src) {
    auto dit = dst.begin();
    auto sit = src.begin();
    while (dit != dst.end() && sit != src.end()) {
        if (sit.key() < dit.key()) {
            ++ sit;
            continue;
        }
        if (dit.key() < sit.key()) {
            ++ dit;
            continue;
        }
        Q_ASSERT(sit.key() == dit.key());
        dit.value() = sit.value();
        ++ sit;
        ++ dit;
    }
    return dst;
}

int main() {
    auto json1 = QJsonDocument::fromJson({R"ZZ({
     "properties":{
        "A":true,
        "B":true,
        "fieldName":"ewfqfqewf",
        "C":false,
        "fieldPassword":"451541611",
        "isBtnSignOnClicked":true
     },
     "type":"xyz"
     })ZZ"}).toVariant().value<QVariantMap>();

    auto json2 = QJsonDocument::fromJson({R"ZZ({
     "A":true,
     "B":true,
     "fieldName":"ewfqfqewf",
     "C":false,
     "fieldPassword":"451541611",
     "isBtnSignOnClicked":true
     })ZZ"}).toVariant().value<QVariantMap>();

    auto json3 = QJsonDocument::fromJson(
    {R"ZZ({
     "fieldName":"nick",
     "fieldPassword":"0000",
     "isBtnSignOnClicked":true
     })ZZ"}).toVariant().value<QVariantMap>();

    json2 = replaceMap(json2, json3);
    auto props = replaceMap(json1["properties"].value<QVariantMap>(), json2);
    json1["properties"] = props;

    qDebug() << QJsonDocument::fromVariant(json1).toJson().constData();
}
