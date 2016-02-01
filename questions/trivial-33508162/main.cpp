// https://github.com/KubaO/stackoverflown/tree/master/questions/trivial-33508162
// main.cpp
#include <QtWidgets>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QWidget widget;
    QVBoxLayout layout(&widget);
    QFrame frame;

    frame.setFrameStyle(QFrame::Panel | QFrame::Plain);
    frame.setLineWidth(5);
    layout.addWidget(&frame);

    widget.setFixedSize(800,600);
    widget.show();
    return a.exec();
}
