#include <QtWidgets>

class WindowMoveMonitor : public QObject
{
   Q_OBJECT
   bool eventFilter(QObject * target, QEvent * event) override {
      if (event->type() == QEvent::Move) {
         auto ev = static_cast<QMoveEvent*>(event);
         emit moved(ev->pos());
      }
      return QObject::eventFilter(target, event);
   }
public:
   WindowMoveMonitor(QWidget * widget, QObject * parent = 0) : QObject{parent}
   {
      Q_ASSERT(widget->window());
      widget->window()->installEventFilter(this);
   }
   Q_SIGNAL void moved(const QPoint &);
};

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   QLabel label{"..."};
   WindowMoveMonitor monitor{&label};
   QObject::connect(&monitor, &WindowMoveMonitor::moved, [&](const QPoint & p){
      label.setText(QStringLiteral("x:%1 y:%2").arg(p.x()).arg(p.y()));
   });
   label.setMinimumSize(200, 50);
   label.show();
   return app.exec();
}

#include "main.moc"
