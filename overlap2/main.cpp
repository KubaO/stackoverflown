//main.cpp
#include <QtGui/QPainter>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QApplication>

class Line : public QWidget
{
public:
    Line(QWidget* parent = 0) : QWidget(parent) {}
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawLine(rect().topLeft(), rect().bottomRight());
    }
};

class Window : public QWidget
{
    QWidget * line;
public:
    Window() {
        QLayout * layout = new QHBoxLayout();
        QLabel * l;
        l = new QLabel("Left", this);
        l->setFrameStyle(QFrame::Box | QFrame::Raised);
        layout->addWidget(l);
        l = new QLabel("Right", this);
        l->setFrameStyle(QFrame::Box | QFrame::Raised);
        layout->addWidget(l);
        setLayout(layout);
        line = new Line(this);
    }
protected:
    void resizeEvent(QResizeEvent *)
    {
        line->setGeometry(rect());
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}

