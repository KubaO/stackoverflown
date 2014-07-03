#include <QMainWindow>
#include <QDockWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDebug>

class Keyboard : public QDockWidget {
    Q_OBJECT
    void init() {
        auto * w = new QWidget;
        auto * l = new QGridLayout(w);
        for (int i = 0; i < 10; ++ i) {
            auto * btn = new QToolButton;
            btn->setText(QString::number(i));
            btn->setProperty("key", Qt::Key_0 + i);
            l->addWidget(btn, 0, i, 1, 1);
            connect(btn, SIGNAL(clicked()), SLOT(clicked()));
        }
        setWidget(w);
        setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        foreach(QObject *o, widget()->children()) {
            if (o->isWidgetType()) static_cast<QWidget*>(o)->setFocusPolicy(Qt::NoFocus);
        }
    }
    void sendKey(Qt::Key key, Qt::KeyboardModifier mod)
    {
        if (! parentWidget()) return;
        QWidget * target = parentWidget()->focusWidget();
        if (! target) return;

        QString repr = QKeySequence(key).toString();
        QKeyEvent *pressEvent = new QKeyEvent(QEvent::KeyPress, key, mod, repr);
        QKeyEvent *releaseEvent = new QKeyEvent(QEvent::KeyRelease, key, mod, repr);
        qApp->postEvent(target, pressEvent);
        qApp->postEvent(target, releaseEvent);
        qDebug() << repr;
    }
    Q_SLOT void clicked() {
        QVariant key = sender()->property("key");
        if (key.isValid()) sendKey((Qt::Key)key.toInt(), Qt::NoModifier);
    }

public:
    explicit Keyboard(const QString & title, QWidget *parent = 0) : QDockWidget(title, parent) { init(); }
    explicit Keyboard(QWidget *parent = 0) : QDockWidget(parent) { init(); }
};

class Window : public QMainWindow {
public:
    Window(QWidget *parent = 0, Qt::WindowFlags f = 0) : QMainWindow(parent, f) {
        setCentralWidget(new QLineEdit);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.addDockWidget(Qt::TopDockWidgetArea, new Keyboard("Keyboard", &w));
    w.show();
    return a.exec();
}

#include "main.moc"
