// https://github.com/KubaO/stackoverflown/tree/master/questions/app-args-35566459
#include <QtCore>

int main(int argc, char ** argv) {
    QScopedPointer<QCoreApplication> app;
    {
        auto & tmp_argc = argc;
        app.reset(new QCoreApplication{tmp_argc, argv});
    }
    QSettings settings{"testvendor"};
    qDebug() << "Num of arguments: " << app->arguments().count();
}
