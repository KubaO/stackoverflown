// https://github.com/KubaO/stackoverflown/tree/master/questions/signal-slot-hierarchy-10783656
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Window : public QWidget
{
   QSignalMapper m_mapper;
   QStackedLayout m_stack{this};
   QWidget      m_page1,                  m_page2;
   QHBoxLayout  m_layout1{&m_page1},      m_layout2{&m_page2};
   QLabel       m_label1{"Page 1"},       m_label2{"Page 2"};
   QPushButton  m_button1{"Show Page 2"}, m_button2{"Show Page 1"};
public:
   Window(QWidget * parent = {}) : QWidget(parent) {
      // the mapper tells the stack which page to switch to
      connect(&m_mapper, SIGNAL(mapped(int)), &m_stack, SLOT(setCurrentIndex(int)));

      // Page 1
      m_layout1.addWidget(&m_label1);
      m_layout1.addWidget(&m_button1);
      // tell the mapper to map signals coming from this button to integer 1 (index of page 2)
      m_mapper.setMapping(&m_button1, 1);
      // when the button is clicked, the mapper will do its mapping and emit the mapped() signal
      connect(&m_button1, SIGNAL(clicked()), &m_mapper, SLOT(map()));
      m_stack.addWidget(&m_page1);

      // Page 2
      m_layout2.addWidget(&m_label2);
      m_layout2.addWidget(&m_button2);
      // tell the mapper to map signals coming from this button to integer 0 (index of page 1)
      m_mapper.setMapping(&m_button2, 0);
      connect(&m_button2, SIGNAL(clicked()), &m_mapper, SLOT(map()));
      m_stack.addWidget(&m_page2);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Window w;
   w.show();
   return a.exec();
}
