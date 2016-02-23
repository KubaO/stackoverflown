// https://github.com/KubaO/stackoverflown/tree/master/questions/app-args-35566459
#include <QtCore>
#include <memory>

std::unique_ptr<QCoreApplication> newApp(int & argc, char ** & argv) {
   return std::unique_ptr<QCoreApplication>(new QCoreApplication{argc, argv});
}

int main(int argc, char ** argv) {
    std::unique_ptr<QCoreApplication> app(newApp(argc, argv));
    QSettings settings{"testvendor"};
    qDebug() << "Num of arguments: " << app->arguments().count();
}
