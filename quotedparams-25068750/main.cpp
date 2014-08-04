#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

static QStringList splitCommandLine(const QString & cmdLine)
{
    QStringList list;
    QString arg;
    bool escape = false;
    enum { Idle, Arg, QuotedArg } state = Idle;
    foreach (QChar const c, cmdLine) {
        if (!escape && c == '\\') { escape = true; continue; }
        switch (state) {
        case Idle:
            if (!escape && c == '"') state = QuotedArg;
            else if (escape || !c.isSpace()) { arg += c; state = Arg; }
            break;
        case Arg:
            if (!escape && c == '"') state = QuotedArg;
            else if (escape || !c.isSpace()) arg += c;
            else { list << arg; arg.clear(); state = Idle; }
            break;
        case QuotedArg:
            if (!escape && c == '"') state = arg.isEmpty() ? Idle : Arg;
            else arg += c;
            break;
        }
        escape = false;
    }
    if (!arg.isEmpty()) list << arg;
    return list;
}

int main(int argc, char * argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption param1("param1");
    QCommandLineOption param2("param2", "", "val2");
    QCommandLineOption param3("param3", "", "val3");
    QCommandLineOption param4("p", "", "val4");
    parser.addOption(param1);
    parser.addOption(param2);
    parser.addOption(param3);
    parser.addOption(param4);
    if (true) {
        // Parse a string
        // The command line reads:
        // /tmp/myprog --param1 --param2=2\ 2 --param3="1\" 2 3" -p 4
        QString cmdLine("/tmp/myprog --param1 --param2=2\\ 2 --param3=\"1\\\" 2 3\" -p 4");
        parser.parse(splitCommandLine(cmdLine));
    } else {
        // Parse a command line passed to this application
        parser.process(app);
    }
    if (parser.isSet(param1)) qDebug() << "param1";
    if (parser.isSet(param2)) qDebug() << "param2:" << parser.value(param2);
    if (parser.isSet(param3)) {
        QStringList values = parser.value(param3)
                .split(' ', QString::SkipEmptyParts);
        qDebug() << "param3:" << values;
    }
    if (parser.isSet(param4)) qDebug() << "param4:" << parser.value(param4);
    return 0;
}
