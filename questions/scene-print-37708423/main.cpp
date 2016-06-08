// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-print-37708423
#include <QtWidgets>
#include <QtPrintSupport>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QGraphicsScene scene;
   QGraphicsView view(&scene);

   auto in = 72.0f;
   auto pen = QPen(Qt::black, 0.01*in);
   QRectF canvasRect(0, 0, 4*in, 4*in);
   // this is to show actual scene
   QGraphicsRectItem sss(canvasRect);
   sss.setPen(pen);
   sss.setBrush(Qt::blue);
   scene.addItem(&sss);
   // this item is partially outside top left
   QGraphicsEllipseItem e1(-0.5*in, -0.5*in, 1*in, 1*in);
   e1.setPen(pen);
   e1.setBrush(Qt::yellow);
   scene.addItem(&e1);
   // this item is partially outside center
   QGraphicsEllipseItem e2(2*in, 2*in, 2.5*in, 1*in);
   e2.setPen(pen);
   e2.setBrush(Qt::yellow);
   scene.addItem(&e2);
   // this item is partially outside right
   QGraphicsEllipseItem e3(3.5*in, 3.5*in, 1*in, 1*in);
   e3.setPen(pen);
   e3.setBrush(Qt::yellow);
   scene.addItem(&e3);

   view.fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
   view.show();

   QPrinter printer;
   QPrintDialog printDialog(&printer);
   QObject::connect(&printDialog, &QDialog::accepted, [&]{
      printer.setOrientation(QPrinter::Landscape);
      QPainter painter(&printer);

      auto source = canvasRect;
      auto scale = printer.resolution()/in;
      auto page = printer.pageRect(QPrinter::DevicePixel);
      auto target = QRectF(page.topLeft(), source.size()*scale);
      target &= page; // clip target rect to page
      qDebug() << page << scale << source << target;
      scene.render(&painter, target, source);
   });
   printDialog.show(); // modal on OS X thus must follow `connect` above
   return app.exec();
}
