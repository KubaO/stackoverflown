// https://github.com/KubaO/stackoverflown/tree/master/questions/rect-paint-42194052
#include <QtWidgets>

class Window : public QWidget
{
   QPointF m_pos{100, 100};
   void paintEvent(QPaintEvent* event) override
   {
      QPainter painter(this);
      painter.setBrush(Qt::red);
      painter.setPen(Qt::black);
      for (int i = 0; i < event->rect().width(); i += 50)
         painter.drawRect(QRectF(m_pos.x() + i, m_pos.y(), 30, 30));
   }
public:
   Window(QWidget *parent = nullptr) : QWidget(parent)
   {
      setAttribute(Qt::WA_NoSystemBackground, true);
      setAttribute(Qt::WA_TranslucentBackground);
      setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                     | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
   }
   void setPos(const QPointF & pos) {
      m_pos = pos;
      update();
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Window w;
   QTimer timer;
   QObject::connect(&timer, &QTimer::timeout, [&w]{
      static bool toggle{};
      if (!w.isVisible()) {
         toggle = !toggle;
         if (toggle)
            w.setPos({200, 200});
         else
            w.setPos({100, 100});
      };
      w.setVisible(!w.isVisible());
   });
   timer.start(500);
   w.resize(1000, 500);
   return app.exec();
}
