#include <QWidget>
#include <QApplication>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QPainter>

#define PropType uint
class Window: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(PropType animProp MEMBER m_animProp NOTIFY animPropChanged)
    PropType m_animProp;
public:
    Window();
    Q_SIGNAL void animPropChanged();
protected:
    void paintEvent(QPaintEvent *);
};

Window::Window()
{
    QPropertyAnimation * anim = new QPropertyAnimation(this, "animProp");
    anim->setDuration( 1000 * 4 );
    anim->setStartValue( 0 );
    anim->setEndValue( 90*100 );
    anim->setLoopCount(-1);

    QPushButton * btn = new QPushButton("Start", this);
    btn->move(20, 20);

    connect(btn, SIGNAL(clicked()), anim, SLOT(start()));
    connect(btn, SIGNAL(clicked()), btn, SLOT(deleteLater()));
    connect(this, SIGNAL(animPropChanged()), SLOT(update()));
}

void Window::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.rotate(m_animProp/100.0);
    p.drawLine(0, 0, width()+height(), 0);
}

int main (int argc, char **argv)
{
    QApplication app(argc, argv);
    Window w;
    w.show();
    return app.exec();
}

#include "main.moc"
