#include <cstdio>
#include <QTextStream>

int main(int, char **)
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    static const uint red_hash = 30900;
    static const uint green_hash = 7244734;
    static const uint blue_hash = 431029;
#else
    static const uint red_hash = 112785;
    static const uint green_hash = 98619139;
    static const uint blue_hash = 3027034;
#endif

    QTextStream in(stdin), out(stdout);
    out << "Enter color: " << flush;
    const QString color = in.readLine();
    out << "Hash=" << qHash(color) << endl;

    QString answer;
    switch (qHash(color)) {
    case red_hash:
        answer="Chose red";
        break;
    case green_hash:
        answer="Chose green";
        break;
    case blue_hash:
        answer="Chose blue";
        break;
    default:
        answer="Chose something else";
        break;
    }
    out << answer << endl;
}
