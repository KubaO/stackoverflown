#include <QtWidgets>
#include <private/qpainter_p.h>
#include <private/qemulationpaintengine_p.h>
#include <qpa/qplatformbackingstore.h>


struct PaintDevice : private QPaintDevice {
   static QPainter * sharedPainter(const QPaintDevice * pd) {
      return static_cast<const PaintDevice*>(pd)->QPaintDevice::sharedPainter();
   }
   static QPainter * sharedPainter(QBackingStore * b) {
      return sharedPainter(b->paintDevice());
   }
};

class Label : public QLabel {
   void paintEvent(QPaintEvent * ev) Q_DECL_OVERRIDE {
      {
         QPainter p(backingStore()->paintDevice());
         QPainter p2(this);
         qDebug() << PaintDevice::sharedPainter(backingStore());
      }
      qDebug() << PaintDevice::sharedPainter(backingStore());

      //backingStore()->paintDevice()->paintEngine();

      QLabel::paintEvent(ev);
   }
public:
   Label(const QString & text, QWidget * parent = 0) : QLabel(text, parent) {}
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Label l1 { "Hello" }, l2 { "Goodbye" };
   for (auto l : { &l1, &l2 }) {
      if (0) l->show();
      l->setMinimumSize(200, 100);
   }
   l1.show();

   auto widgetEngine = dynamic_cast<QPaintEngineEx*>(l1.backingStore()->paintDevice()->paintEngine());
   //auto platformStore = dynamic_cast<QPlatformBackingStore*>(l1.backingStore());
   //qDebug() << widgetEngine << platformStore;
   //QEmulationPaintEngine engine(widgetEngine);



   //QPaintEngineState newState;
   //newState.
   //qDebug() << l1.backingStore()->paintDevice()->paintEngine()->painter();


   return a.exec();
}
