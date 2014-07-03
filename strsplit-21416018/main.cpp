#include <QBuffer>
#include <QTextStream>
#include <QStringList>
#include <QMultiMap>
#include <QDebug>

QByteArray fileData(
        "Brandan Janurary 1 2000 Math 7A 4 9 10 9 10 9 0 0 0 0 0 0\n"
        "Brandan Janurary 1 2000 Math 7A 4 9 10 7 10 10 0 0 0 0 0 0\n"
        "Brandan Janurary 1 2000 Math 7A 4 10 10 10 10 10 0 0 0 0 0 0\n"
        "Bob Janurary 1 2000 Math 7A 4 9 8 10 10 10 0 0 0 0 0 0\n"
        "Bob Janurary 1 2000 Math 7A 4 9 8 10 10 10 0 0 0 0 0 0");

void processData(QIODevice * dev)
{
    if (! dev->open(QIODevice::ReadOnly | QIODevice::Text)) return;

    typedef QMultiMap<QString, QStringList> Map;
    Map map;

    QTextStream stream(dev);
    while(! stream.atEnd()) {
        QString line = stream.readLine();
        QStringList elements = line.split(' ', QString::SkipEmptyParts);
        if (elements.count() < 2) continue;
        QString name = elements[0];
        map.insert(name, elements);
    }

    for (Map::const_iterator it = map.begin(); it != map.end(); ++it) {
        qDebug() << it.key() << it.value()[1];
    }
}

int main()
{
    QBuffer buf(&fileData);
    processDataOrg(&buf);
}

void processDataOrg(QIODevice * dev)
{
    if (!dev->open(QIODevice::ReadOnly | QIODevice::Text)) return;

    const int N = 10000;
    QScopedArrayPointer<QString> listof(new QString[N]);
    QTextStream stream(dev);
    int x = -1; /* FIX for problem #2 */
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        listof[++x] = line;  /* FIX for problem #2 */
        if (x == N-1) break;
    }

    QScopedArrayPointer<QString> reallist(new QString[N]);
    for (int q=x; q>=0; q--) {
        QString Parsing = listof[q];
        QStringList listt = Parsing.split(" ", QString::SkipEmptyParts);
        if (listt.size() < 2) continue; /* FIX for problem #1 */

        qDebug() << listt[0] << listt[1];
        reallist[q] = listt[0];
    }
}

void processDataBetter(QIODevice * dev)
{
    if (!dev->open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QStringList lines;
    QTextStream stream(dev);
    while (! stream.atEnd()) {
        lines << stream.readLine();;
    }

    QStringList names;
    foreach (QString line, lines) {
        QStringList elements = line.split(' ');
        qDebug() << elements[0] << elements[1];
        names << elements[0];
    }
}

void processDataOK(QIODevice * dev)
{
    if (!dev->open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QStringList names;
    QTextStream stream(dev);
    while (! stream.atEnd()) {
        QString line = stream.readLine();
        QStringList elements = line.split(' ', QString::SkipEmptyParts);
        if (elements.count() < 2) continue;
        qDebug() << elements[0] << elements[1];
        names << elements[0];
    }
}
