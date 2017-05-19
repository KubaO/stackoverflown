// https://github.com/KubaO/stackoverflown/tree/master/questions/whatsthis-bypass-44073556
#include <QtWidgets>

// See http://stackoverflow.com/a/32027028/1329652
class WhatsThisSignaler : public QObject {
   Q_OBJECT
   bool eventFilter(QObject * obj, QEvent * ev) override {
      if (!obj->isWidgetType())
         return false;
      auto widget = static_cast<QWidget*>(obj);
      if (!widget->isWindow())
         return false;
      switch (ev->type()) {
      case QEvent::EnterWhatsThisMode:
      case QEvent::LeaveWhatsThisMode:
         emit whatsThisEvent(widget, ev);
         break;
      default:
         break;
      }
      return false;
   }
public:
   Q_SIGNAL void whatsThisEvent(QWidget *, QEvent *);
   WhatsThisSignaler(QObject * parent = {}) : QObject(parent) {}
   void installOn(QWidget * widget) {
      widget->installEventFilter(this);
   }
   void installOn(QCoreApplication * app) {
      app->installEventFilter(this);
   }
};

// See http://stackoverflow.com/q/22535469/1329652
template<typename EnumType> QString toName(EnumType enumValue)
{
    auto * enumName = qt_getEnumName(enumValue);
    auto * metaObject = qt_getEnumMetaObject(enumValue);
    QString name;
    if (metaObject) {
        auto enumIndex = metaObject->indexOfEnumerator(enumName);
        name = metaObject->enumerator(enumIndex).valueToKey(enumValue);
    }
    if (name.isEmpty())
       name = QString::number((int)enumValue);
    return name;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   WhatsThisSignaler sig;
   QPlainTextEdit w;
   w.setWindowFlags(Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint);
   w.setMinimumSize(200, 200);
   w.setReadOnly(true);
   w.show();
   sig.installOn(&w);
   QObject::connect(&sig, &WhatsThisSignaler::whatsThisEvent, &w, [&w](QWidget*widget, QEvent*ev){
      w.appendPlainText(QStringLiteral("%1(0x%2) \"%3\" QEvent::%4")
                        .arg(widget->metaObject()->className())
                        .arg((uintptr_t)widget, 0, 16)
                        .arg(widget->objectName())
                        .arg(toName(ev->type())));
   });
   return app.exec();
}

#include "main.moc"
