#include <QApplication>
#include <QGridLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QTableWidget>
#include <QPushButton>

class Widget : public QWidget {
    QGridLayout m_layout;
    QGroupBox m_group1;
    QGridLayout m_group1Layout;
    QGroupBox m_group2;
    QGridLayout m_group2Layout;
    QGroupBox m_group3;
    QGridLayout m_group3Layout;
public:
    Widget(QWidget * parent = 0) : QWidget(parent),
        m_layout(this),
        m_group1("First Group"),
        m_group1Layout(&m_group1),
        m_group2("Second Group"),
        m_group2Layout(&m_group2),
        m_group3("Third Group"),
        m_group3Layout(&m_group3)
    {
        m_layout.addWidget(&m_group1, 0, 0, 1, 2);
        m_layout.addWidget(&m_group2, 1, 0);
        m_layout.addWidget(&m_group3, 1, 1);

        // Line edits in group 1
        for (int i = 0; i < 3; ++ i)
            m_group1Layout.addWidget(new QLineEdit, 0, i);

        // Table and buttons in group 2
        m_group2Layout.addWidget(new QTableWidget, 0, 0, 4, 1);
        for (int i = 0; i < 4; ++ i)
            m_group2Layout.addWidget(new QPushButton(QString::number(i)), i, 1);

        // Table and buttons in group 3
        m_group3Layout.addWidget(new QTableWidget, 0, 0, 4, 1);
        for (int i = 0; i < 4; ++ i)
            m_group3Layout.addWidget(new QPushButton(QString::number(i)), i, 1);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
