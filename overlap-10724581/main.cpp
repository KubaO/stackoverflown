//main.cpp
#include <QtGui/QLabel>
#include <QHBoxLayout>
#include <QtGui/QApplication>

class TopLabel : public QLabel
{
    Q_OBJECT
public:
    TopLabel(QWidget * parent = 0) : QLabel(parent) {}
    TopLabel(const QString & text, QWidget * parent = 0) : QLabel(text, parent) {}
public slots:
    void bottomGeometry(const QRect & r) {
        setGeometry(r.left() + r.width()*0.25, r.top() + r.height()*0.1,
                    r.width()*0.5, r.height()*0.5);
    }
};

class BottomLabel : public QLabel
{
    Q_OBJECT
public:
    BottomLabel(QWidget * parent = 0) : QLabel(parent) {}
    BottomLabel(const QString & text, QWidget * parent = 0) : QLabel(text, parent) {}
signals:
    void newGeometry(const QRect & r);
protected:
    void resizeEvent(QResizeEvent *) { emit newGeometry(geometry()); }
    void moveEvent(QMoveEvent *) { emit newGeometry(geometry()); }
};

class Window : public QWidget
{
public:
    Window() {
        QLayout * layout = new QHBoxLayout();
        QLabel * l = new QLabel("Left", this);
        l->setFrameStyle(QFrame::Box | QFrame::Raised);
        layout->addWidget(l);
        BottomLabel * bl = new BottomLabel("Right", this);
        bl->setFrameStyle(QFrame::Box | QFrame::Raised);
        TopLabel * tl = new TopLabel("TOP", this);
        tl->setFrameStyle(QFrame::StyledPanel);
        connect(bl, SIGNAL(newGeometry(QRect)), tl, SLOT(bottomGeometry(QRect)));
        layout->addWidget(bl);
        setLayout(layout);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
