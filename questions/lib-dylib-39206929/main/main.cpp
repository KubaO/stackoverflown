// https://github.com/KubaO/stackoverflown/tree/master/questions/lib-dylib-39206929
//### main/main.cpp
#include "lib1/lib1.h"
#include <QtWidgets>

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    Lib1 lib1;
    QLabel label{lib1.text()};
    label.setMinimumSize(200, 200);
    label.setFont(QFont{"Helvetica", 20});
    label.show();
    return app.exec();
}
