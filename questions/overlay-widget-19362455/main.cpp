// https://github.com/KubaO/stackoverflown/tree/master/questions/overlay-widget-19362455
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class OverlayWidget : public QWidget
{
   void newParent() {
      if (!parent()) return;
      parent()->installEventFilter(this);
      raise();
   }
public:
   explicit OverlayWidget(QWidget * parent = {}) : QWidget{parent} {
      setAttribute(Qt::WA_NoSystemBackground);
      setAttribute(Qt::WA_TransparentForMouseEvents);
      newParent();
   }
protected:
   //! Catches resize and child events from the parent widget
   bool eventFilter(QObject * obj, QEvent * ev) override {
      if (obj == parent()) {
         if (ev->type() == QEvent::Resize)
            resize(static_cast<QResizeEvent*>(ev)->size());
         else if (ev->type() == QEvent::ChildAdded)
            raise();
      }
      return QWidget::eventFilter(obj, ev);
   }
   //! Tracks parent widget changes
   bool event(QEvent* ev) override {
      if (ev->type() == QEvent::ParentAboutToChange) {
         if (parent()) parent()->removeEventFilter(this);
      }
      else if (ev->type() == QEvent::ParentChange)
         newParent();
      return QWidget::event(ev);
   }
};

class LoadingOverlay : public OverlayWidget
{
public:
   LoadingOverlay(QWidget * parent = {}) : OverlayWidget{parent} {
      setAttribute(Qt::WA_TranslucentBackground);
   }
protected:
   void paintEvent(QPaintEvent *) override {
      QPainter p{this};
      p.fillRect(rect(), {100, 100, 100, 128});
      p.setPen({200, 200, 255});
      p.setFont({"arial,helvetica", 48});
      p.drawText(rect(), "Loading...", Qt::AlignHCenter | Qt::AlignVCenter);
   }
};

int main(int argc, char * argv[])
{
   QApplication a{argc, argv};
   QMainWindow window;
   QLabel central{"Hello"};
   central.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   central.setMinimumSize(400, 300);
   LoadingOverlay overlay{&central};
   QTimer::singleShot(5000, &overlay, SLOT(hide()));
   window.setCentralWidget(&central);
   window.show();
   return a.exec();
}
