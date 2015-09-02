#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>
#include <QFrame>

class Widget : public QWidget {
    mutable int m_ctr;
public:
    Widget(QWidget *parent = 0) : QWidget(parent), m_ctr(0) {
        QSizePolicy p(QSizePolicy::Expanding, QSizePolicy::Minimum);
        p.setHeightForWidth(true);
        setSizePolicy(p);
    }
    int heightForWidth(int width) const {
        m_ctr ++;
        QApplication::postEvent(const_cast<Widget*>(this), new QEvent(QEvent::UpdateRequest));
        return width;
    }
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
        p.drawText(rect(), QString("h4w called %1 times").arg(m_ctr));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    QVBoxLayout * l = new QVBoxLayout(&w);
    l->addWidget(new Widget);
    QFrame * btm = new QFrame;
    btm->setFrameShape(QFrame::Panel);
    btm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->addWidget(btm);
    w.show();
    return a.exec();
}
