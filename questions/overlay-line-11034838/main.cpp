// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-line-11034838
#include <QtGui>
#if QT_VERSION > QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Line : public QWidget {
    void paintEvent(QPaintEvent *) override {
        QPainter p{this};
        p.setRenderHint(QPainter::Antialiasing);
        p.drawLine(rect().topLeft(), rect().bottomRight());
    }
public:
    using QWidget::QWidget;
};

class Window : public QWidget {
    QHBoxLayout layout{this};
    QLabel left{"Left", this};
    QLabel right{"Right", this};
    Line line{this};

    void resizeEvent(QResizeEvent *) override {
        line.setGeometry(rect());
    }
public:
    Window(QWidget * parent = nullptr) : QWidget{parent} {
        left.setFrameStyle(QFrame::Box | QFrame::Raised);
        layout.addWidget(&left);
        right.setFrameStyle(QFrame::Box | QFrame::Raised);
        layout.addWidget(&right);
    }
};

int main(int argc, char *argv[])
{
    QApplication app{argc, argv};
    Window w;
    w.show();
    return app.exec();
}
