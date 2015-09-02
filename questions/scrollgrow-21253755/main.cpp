#include <QApplication>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QGridLayout>

class XYZ {
public:
    operator QObject*() { return 0; }
};

class ButtonGroup : public QWidget {
    Q_OBJECT
public:
    ButtonGroup(QWidget * parent = 0) : QWidget(parent) {
        new QHBoxLayout(this);
        layout()->setSizeConstraint(QLayout::SetMinAndMaxSize); // <<< Essential
    }
    Q_SLOT void addButton() {
        int n = layout()->count();
        layout()->addWidget(new QPushButton(QString("Btn #%1").arg(n+1)));
    }
};

class AdjustingScrollArea : public QScrollArea {
    bool eventFilter(QObject * obj, QEvent * ev) {
        if (obj == widget() && ev->type() != QEvent::Resize) {
            // Essential vvv
            setMaximumWidth(width() - viewport()->width() + widget()->width());
        }
        return QScrollArea::eventFilter(obj, ev);
    }
public:
    AdjustingScrollArea(QWidget * parent = 0) : QScrollArea(parent) {}
    void setWidget(QWidget *w) {
        QScrollArea::setWidget(w);
        // It so happens that QScrollArea already filters widget events,
        // but that's an implementation detail that we shouldn't rely on.
        w->installEventFilter(this);
    }
};

class Window : public QWidget {
    Q_OBJECT
public:
    Window() {
        QGridLayout * layout = new QGridLayout(this);
        QWidget * w;
        QScrollArea * area;
        ButtonGroup * group;

        layout->addWidget(w = new QLabel(">>"), 0, 0);
        w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        w->setStyleSheet("border: 1px solid green");

        layout->addWidget(area = new AdjustingScrollArea, 0, 1);
        area->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        area->setStyleSheet("QScrollArea { border: 1px solid blue }");
        area->setWidget(group = new ButtonGroup);
        layout->setColumnStretch(1, 1);

        layout->addWidget(w = new QLabel("<<"), 0, 2);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        w->setStyleSheet("border: 1px solid green");

        layout->addWidget(w = new QPushButton("Add a widget"), 1, 0, 1, 3);
        connect(w, SIGNAL(clicked()), group, SLOT(addButton()));
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
