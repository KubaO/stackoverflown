// https://github.com/KubaO/stackoverflown/tree/master/questions/widget-show-32534931
#include <QtWidgets>

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QWidget w;
    w.setMinimumSize(200, 50);
    QLabel visible{"Visible", &w};
    w.show();
    QLabel invisible{"Invisible", &w};
    invisible.move(100, 0);
    return app.exec();
}

