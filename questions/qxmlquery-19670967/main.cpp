#include <QXmlQuery>
#include <QBuffer>
#include <QTextStream>
#include <QApplication>
#include <QDebug>

const char xmlData[]=
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<plist version=\"1.0\"><dict>"
        "<key>BuildMachineOSBuild</key><string>13A598</string>"
        "<key>CFBundleShortVersionString</key><string>1.4</string>"
        "<key>CFBundleSignature</key><string>????</string>"
        "<key>CFBundleVersion</key><string>1.4</string>"
        "<key>NSPrincipalClass</key><string>NSApplication</string>"
        "</dict></plist>";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextStream out(stdout);
    QByteArray data(QByteArray::fromRawData(xmlData, sizeof(xmlData)-1));
    QBuffer buffer(&data);
    if (buffer.open(QIODevice::ReadOnly)) {
        QString version;
        QXmlQuery query;
        query.bindVariable("file", &buffer);
        query.setQuery("declare variable $file external; doc($file)/plist/dict/key[node()='CFBundleShortVersionString']/following-sibling::string[1]/node()");
        bool rc = query.evaluateTo(&version);
        qDebug() << rc << version;
    }
}
