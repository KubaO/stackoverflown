#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QMetaProperty>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

QJsonObject serializeDialog(const QWidget * dialog) {
    QJsonObject json;
    foreach (QWidget * widget, dialog->findChildren<QWidget*>()) {
        if (widget->objectName().isEmpty()) continue;
        QMetaProperty prop = widget->metaObject()->userProperty();
        if (! prop.isValid()) continue;
        QJsonValue val(QJsonValue::fromVariant(prop.read(widget)));
        if (val.isUndefined()) continue;
        json.insert(widget->objectName(), val);
    }
    return json;
}

class MyDialog : public QWidget {
    QTextEdit *notebookid, *tagid;
public:
    MyDialog(QWidget * parent = 0) : QWidget(parent),
        notebookid(new QTextEdit(this)),
        tagid(new QTextEdit(this))
    {

    }
    QString getJson() const {
        QJsonDocument doc(serializeDialog(this));
        return QString::fromUtf8(doc.toJson());
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    return a.exec();
}
