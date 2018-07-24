// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-line-11034838
#include <QtGui>
#if QT_VERSION > QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Line : public QWidget {
protected:
   void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawLine(rect().topLeft(), rect().bottomRight());
    }
public:
    explicit Line(QWidget *parent = nullptr) : QWidget(parent) {
       setAttribute(Qt::WA_TransparentForMouseEvents);
    }
};

class Window : public QWidget {
    QHBoxLayout layout{this};
    QPushButton left{"Left"};
    QLabel right{"Right"};
    Line line{this};
protected:
    void resizeEvent(QResizeEvent *) override {
        line.resize(size());
    }
public:
    explicit Window(QWidget * parent = nullptr) : QWidget(parent) {
        layout.addWidget(&left);
        right.setFrameStyle(QFrame::Box | QFrame::Raised);
        layout.addWidget(&right);
        line.raise();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Window w;
    w.show();
    return app.exec();
}
