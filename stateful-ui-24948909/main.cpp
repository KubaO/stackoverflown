#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QStateMachine>
#include <QGridLayout>

class Widget : public QWidget {
    QGridLayout m_layout;
    QLabel m_label1, m_label2, m_label3;
    QPushButton m_button1, m_button2, m_button3;
    QStateMachine m_machine;
    QState m_editState, m_boldState, m_edit1, m_edit2, m_boldOn, m_boldOff;
public:
    Widget(QWidget * parent = 0) : QWidget(parent), m_layout(this),
        m_label1("--"), m_label2("--"), m_label3("--"),
        m_button1("Edit State 1"), m_button2("Edit State 2"), m_button3("Toggle Bold State"),
        m_editState(&m_machine), m_boldState(&m_machine),
        m_edit1(&m_editState), m_edit2(&m_editState),
        m_boldOn(&m_boldState), m_boldOff(&m_boldState)
    {
        m_layout.addWidget(&m_label1, 0, 0);
        m_layout.addWidget(&m_label2, 0, 1);
        m_layout.addWidget(&m_label3, 0, 2);
        m_layout.addWidget(&m_button1, 1, 0);
        m_layout.addWidget(&m_button2, 1, 1);
        m_layout.addWidget(&m_button3, 1, 2);

        m_edit1.assignProperty(&m_label1, "text", "Edit State 1");
        m_edit2.assignProperty(&m_label2, "text", "Edit State 2");
        m_boldOn.assignProperty(&m_label3, "text", "Bold On");
        m_boldOff.assignProperty(&m_label3, "text", "Bold Off");

        m_editState.setInitialState(&m_edit1);
        m_boldState.setInitialState(&m_boldOff);

        foreach (QState * s, QList<QState*>() << &m_edit1 << &m_edit2) {
            s->addTransition(&m_button1, SIGNAL(clicked()), &m_edit1);
            s->addTransition(&m_button2, SIGNAL(clicked()), &m_edit2);
        }
        m_boldOn.addTransition(&m_button3, SIGNAL(clicked()), &m_boldOff);
        m_boldOff.addTransition(&m_button3, SIGNAL(clicked()), &m_boldOn);

        m_machine.setGlobalRestorePolicy(QState::RestoreProperties);
        m_machine.setChildMode(QState::ParallelStates);
        m_machine.start();
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
