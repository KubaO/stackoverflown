// https://github.com/KubaO/stackoverflown/tree/master/questions/scroll-18703286
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QApplication>

class Window : public QWidget
{
   QVBoxLayout m_layout{this};
   QScrollArea m_area;
   QWidget m_contents;
   QVBoxLayout m_contentsLayout{&m_contents};
   QSpinBox m_spinBoxes[10];
public:
   Window(QWidget *parent = {}) : QWidget(parent) {
      m_layout.addWidget(&m_area);
      m_area.setWidget(&m_contents);
      for (auto & spinbox : m_spinBoxes)
         m_contentsLayout.addWidget(&spinbox);
      m_contentsLayout.setSizeConstraint(QLayout::SetMinimumSize);
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   Window w;
   w.show();
   return app.exec();
}
