#include <QScrollArea>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QApplication>

class Window : public QWidget
{
   QVBoxLayout m_mainLayout;
   QScrollArea m_area;
public:
    Window(QWidget *parent = 0) : QWidget(parent), m_mainLayout(this)
    {
        m_mainLayout.addWidget(&m_area);
        QWidget * contents = new QWidget;
        QVBoxLayout * layout = new QVBoxLayout(contents);
        for (int i = 0; i < 10; i++) {
            layout->addWidget(new QSpinBox);
        }
        layout->setSizeConstraint(QLayout::SetMinimumSize);
        m_area.setWidget(contents);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window w;
    w.show();
    return app.exec();
}
