// https://github.com/KubaO/stackoverflown/tree/master/questions/signal-slot-hierarchy-10783656
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Window : public QWidget
{
public:
    Window() : QWidget() {
        QSignalMapper * mapper = new QSignalMapper(this);
        QWidget * page, * button;
        QLayout * layout;

        QStackedLayout * stack = new QStackedLayout;
        // the mapper tells the stack which page to switch to
        connect(mapper, SIGNAL(mapped(int)), stack, SLOT(setCurrentIndex(int)));

        // Page 1
        page = new QWidget(this);
        layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Page 1", page));
        button = new QPushButton("Show Page 2", page);
        // tell the mapper to map signals coming from this button to integer 1 (index of page 2)
        mapper->setMapping(button, 1);
        // when the button is clicked, the mapper will do its mapping and emit the mapped() signal
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
        page->setLayout(layout);
        stack->addWidget(page);

        // Page 2
        page = new QWidget(this);
        layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Page 2", page));
        button = new QPushButton("Show Page 1", page);
        // tell the mapper to map signals coming from this button to integer 0 (index of page 1)
        mapper->setMapping(button, 0);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
        page->setLayout(layout);
        stack->addWidget(page);

        setLayout(stack);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}
