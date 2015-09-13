// https://github.com/KubaO/stackoverflown/tree/master/questions/signal-nice-32539730
#include <QtCore>
#include "posixsignalproxy.h"

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    PosixSignalProxy proxy{[]{}, 2};
    return app.exec();
}

