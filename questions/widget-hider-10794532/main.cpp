#include <QEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QApplication>

class Hider : public QObject
{
    Q_OBJECT
public:
    Hider(QObject * parent = 0) : QObject(parent) {}
    bool eventFilter(QObject *, QEvent * ev) {
        return ev->type() == QEvent::Paint;
    }
    void hide(QWidget * w) {
        w->installEventFilter(this);
        w->update();
    }
    void unhide(QWidget * w) {
        w->removeEventFilter(this);
        w->update();
    }
    Q_SLOT void hideWidget()
    {
        QObject * s = sender();
        if (s->isWidgetType()) { hide(qobject_cast<QWidget*>(s)); }
    }
};

class Window : public QWidget
{
    Q_OBJECT
    Hider m_hider;
    QDialogButtonBox m_buttons;
    QWidget * m_widget;
    Q_SLOT void on_hide_clicked() { m_hider.hide(m_widget); }
    Q_SLOT void on_show_clicked() { m_hider.unhide(m_widget); }
public:
    Window() {
        QGridLayout * lt = new QGridLayout(this);
        lt->addWidget(new QLabel("label1"), 0, 0);
        lt->addWidget(m_widget = new QLabel("hiding label2"), 0, 1);
        lt->addWidget(new QLabel("label3"), 0, 2);
        lt->addWidget(&m_buttons, 1, 0, 1, 3);
        QWidget * b;
        b = m_buttons.addButton("&Hide", QDialogButtonBox::ActionRole);
        b->setObjectName("hide");
        b = m_buttons.addButton("&Show", QDialogButtonBox::ActionRole);
        b->setObjectName("show");
        b = m_buttons.addButton("Hide &Self", QDialogButtonBox::ActionRole);
        connect(b, SIGNAL(clicked()), &m_hider, SLOT(hideWidget()));
        QMetaObject::connectSlotsByName(this);
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
