#include <QTextStream>
#include <QLocale>
int main(int, char **)
{
    QTextStream out(stdout);
    QLocale sysLoc = QLocale::system();
    double value = 99999999999999999999999999999999.;
    double max = 999999999999999999.99;
    int decimals = 2;
    //Q_ASSERT(value < max);
    QString str = sysLoc.toString(value, 'f', decimals);
    str.remove(sysLoc.groupSeparator());
    out << str << endl;
    return 0;
}
