// https://github.com/KubaO/stackoverflown/tree/master/questions/scrollgrow-21253755
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class ButtonGroup : public QWidget {
   Q_OBJECT
   QHBoxLayout m_layout{this};
public:
   ButtonGroup(QWidget * parent = 0) : QWidget{parent} {
      m_layout.setSizeConstraint(QLayout::SetMinAndMaxSize); // <<< Essential
   }
   Q_SLOT void addButton() {
      auto n = m_layout.count();
      m_layout.addWidget(new QPushButton{QString{"Btn #%1"}.arg(n+1)});
   }
};

class AdjustingScrollArea : public QScrollArea {
   bool eventFilter(QObject * obj, QEvent * ev) {
      if (obj == widget() && ev->type() == QEvent::Resize) {
         // Essential vvv
         setMaximumWidth(width() - viewport()->width() + widget()->width());
      }
      return QScrollArea::eventFilter(obj, ev);
   }
public:
   AdjustingScrollArea(QWidget * parent = 0) : QScrollArea{parent} {}
   void setWidget(QWidget *w) {
      QScrollArea::setWidget(w);
      // It happens that QScrollArea already filters widget events,
      // but that's an implementation detail that we shouldn't rely on.
      w->installEventFilter(this);
   }
};

class Window : public QWidget {
   QGridLayout         m_layout{this};
   QLabel              m_left{">>"};
   AdjustingScrollArea m_area;
   QLabel              m_right{"<<"};
   QPushButton         m_add{"Add a widget"};
   ButtonGroup         m_group;
public:
   Window() {
      m_layout.addWidget(&m_left, 0, 0);
      m_left.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
      m_left.setStyleSheet("border: 1px solid green");

      m_layout.addWidget(&m_area, 0, 1);
      m_area.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
      m_area.setStyleSheet("QScrollArea { border: 1px solid blue }");
      m_area.setWidget(&m_group);
      m_layout.setColumnStretch(1, 1);

      m_layout.addWidget(&m_right, 0, 2);
      m_right.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      m_right.setStyleSheet("border: 1px solid green");

      m_layout.addWidget(&m_add, 1, 0, 1, 3);
      connect(&m_add, SIGNAL(clicked()), &m_group, SLOT(addButton()));
   }
};

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   Window w;
   w.show();
   return a.exec();
}

#include "main.moc"
