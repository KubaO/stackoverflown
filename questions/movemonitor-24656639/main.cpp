#include <QApplication>
#include <QLabel>
#include <QMoveEvent>

class WindowMoveMonitor : public QObject
{
   Q_OBJECT
   bool eventFilter(QObject * target, QEvent * event) {
      if (event->type() == QEvent::Move) {
         QMoveEvent * ev = static_cast<QMoveEvent*>(event);
         emit windowMoved(ev->pos());
      }
      return QObject::eventFilter(target, event);
   }
public:
   WindowMoveMonitor(QWidget * widget, QObject * parent = 0) : QObject(parent)
   {
      Q_ASSERT(widget->window());
      widget->window()->installEventFilter(this);
   }
   Q_SIGNAL void windowMoved(const QPoint &);
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QLabel label("...");
   WindowMoveMonitor monitor(&label);
   QObject::connect(&monitor, &WindowMoveMonitor::windowMoved, [&label](const QPoint & p){
      label.setText(QString("x:%1 y:%2").arg(p.x()).arg(p.y()));
   });
   label.setMinimumSize(200, 50);
   label.show();
   return a.exec();
}

#include "main.moc"
