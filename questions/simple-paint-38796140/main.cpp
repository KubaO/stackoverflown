// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-paint-38796140
#include <QtWidgets>

class CustomWidget : public QWidget
{
   QPoint m_mousePos;
public:
   explicit CustomWidget(QWidget *parent = nullptr) : QWidget{parent} {}
   void paintEvent(QPaintEvent *) override;
   void mouseMoveEvent(QMouseEvent *event) override {
      m_mousePos = event->pos();
      update();
   }
};

void CustomWidget::paintEvent(QPaintEvent *)
{
   QPainter painter(this);

   auto r1 = rect().adjusted(10, 10, -10, -10);
   painter.setPen(Qt::white);
   painter.drawRect(r1);

   auto r2 = QRect{QPoint(0, 0), QSize(100, 100)};
   r2.moveCenter(m_mousePos);
   painter.setPen(QPen{Qt::black, 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin});
   painter.drawRect(r2);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   CustomWidget w;
   w.show();
   return app.exec();
}
