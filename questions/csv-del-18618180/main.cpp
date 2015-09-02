#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <cstdio>

QTextStream err(stderr);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if (a.arguments().length() != 3) {
        err << "Usage: " << a.arguments()[0] << " infile outfile" << endl;
        return 1;
    }
    QFile fin(a.arguments()[1]), fout(a.arguments()[2]);
    if (! fin.open(QIODevice::ReadOnly)) {
        err << "Can't open input file: " << fin.fileName() << endl;
        return 2;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        err << "Can't open output file: " << fout.fileName() << endl;
        return 3;
    }
    QTextStream in(&fin), out(&fout);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.endsWith(",")) line.truncate(line.length()-1);
        out << line << "\n";
        if (in.status() != QTextStream::Ok) {
            err << "Error while reading." << endl;
            return 4;
        }
        if (out.status() != QTextStream::Ok) {
            err << "Error while writing." << endl;
            return 5;
        }
    }
    return 0;
}
