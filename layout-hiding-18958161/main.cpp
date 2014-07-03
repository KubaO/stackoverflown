#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QStateMachine>
#include <QPlainTextEdit>
#include <QPointer>

static const int N = 4;
static QPointer<QPlainTextEdit> logView;

class State : public QState
{
public:
    explicit State(const QString& name, QState* parent = 0) : QState(parent) {
        setObjectName(name);
    }
protected:
    virtual void onEntry(QEvent*) {
        QString state = objectName();
        QState* parent = this;
        while ((parent = parent->parentState()) && !parent->objectName().isEmpty() )
        {
            state = parent->objectName() + "->" + state;
        }
        logView->appendHtml(QString("<font color=\"green\">Entering state: <b>%1</b></font>").arg(state));
    }
    virtual void onExit(QEvent*) {
        QString state = objectName();
        QState* parent = this;
        while ((parent = parent->parentState()) && !parent->objectName().isEmpty() )
        {
            state = parent->objectName() + "->" + state;
        }
        logView->appendHtml(QString("<font color=\"red\">Exiting state: <b>%1</b></font>").arg(state));
    }
};

class Widget : public QWidget {
public:
    explicit Widget(bool qt4) {
        QGridLayout *vert = new QGridLayout(this);
        QGridLayout *layout = new QGridLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        logView = new QPlainTextEdit;
        logView->appendPlainText(QString("Qt %1").arg(QT_VERSION_STR));
        vert->addLayout(layout, 0, 0, 1, 1);
        vert->addWidget(logView, 1, 0, 1, 1);
        QStateMachine * const machine = new QStateMachine(this);
        machine->setObjectName("machine");
        State * const all = new State("all", machine);
        State * const none = new State("none", machine);
        QList<QState*> ones;
        for (int i = 0; i < N; ++ i) {
            const QString label = QString("View %1").arg(i+1);
            ones << new State(label, none);
        }
        for (int i = 0; i < N; ++ i) {
            QState *one = ones[i];
            QGraphicsView *view = new QGraphicsView;
            QGraphicsScene *scene = new QGraphicsScene(view);
            scene->addText(one->objectName());
            view->setScene(scene);
            layout->addWidget(view, 2*(i/2), i%2, 1, 1);
            QPushButton *button = new QPushButton(one->objectName());
            layout->addWidget(button, 2*(i/2)+1, i%2, 1, 1);
            button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
            all->assignProperty(view, "visible", true);
            all->assignProperty(button, "visible", true);
            if (qt4) {
                // Workaround for a bug: properties in nested states are
                // sometimes not set correctly, so we explicitly set all properties
                // in one state.
                foreach (QState* s, ones) {
                    s->assignProperty(view, "visible", s == one);
                    s->assignProperty(button, "visible", s == one);
                }
            } else {
                none->assignProperty(view, "visible", false);
                none->assignProperty(button, "visible", false);
                one->assignProperty(view, "visible", true);
                one->assignProperty(button, "visible", true);
            }
            all->addTransition(button, SIGNAL(clicked()), one);
            one->addTransition(button, SIGNAL(clicked()), all);
            if (!none->initialState()) none->setInitialState(one);
        }
        machine->setInitialState(all);
        machine->start();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w(QT_VERSION < QT_VERSION_CHECK(5,0,0));
    w.show();
    return a.exec();
}

