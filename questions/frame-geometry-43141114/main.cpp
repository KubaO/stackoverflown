// https://github.com/KubaO/stackoverflown/tree/master/questions/frame-geometry-43141114
#include <QtWidgets>

struct Client : QWidget {
   void paintEvent(QPaintEvent*) override {
      QPainter p{this};
      p.setBrush({Qt::yellow});
      p.setPen(Qt::NoPen);
      p.drawRect(rect());
   }
};

class FrameMonitor : public QWidget {
   QRect m_clientArea;
   bool eventFilter(QObject *watched, QEvent *event) override {
      Q_ASSERT(watched->isWidgetType());
      if (event->type() == QEvent::Resize || event->type() == QEvent::Move
          || event->type() == QEvent::ActivationChange)
         react(static_cast<QWidget*>(watched));
      else if (event->type() == QEvent::Show) {
         react(static_cast<QWidget*>(watched));
         show();
      } else if (event->type() == QEvent::Hide)
         hide();
      else if (event->type() == QEvent::Close)
         close();
      return false;
   }
   void react(QWidget * w) {
      setGeometry(w->frameGeometry().adjusted(-1, -1, 1, 1));
      m_clientArea = QRect{mapFromGlobal(w->geometry().topLeft()), w->geometry().size()};
      update();
   }
   void paintEvent(QPaintEvent *) override {
      QPainter p{this};
      p.setBrush(Qt::transparent);
      p.setPen(Qt::red);
      p.drawRect(rect().adjusted(0, 0, -1, -1));
      p.setPen(Qt::blue);
      p.setBrush({Qt::blue, Qt::BDiagPattern});
      p.drawRect(m_clientArea.adjusted(0, 0, -1, -1));
   }
public:
   FrameMonitor(QWidget * parent) :
      QWidget{parent, Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint
              | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput} {
      parent->installEventFilter(this);
      setAttribute(Qt::WA_TranslucentBackground);
      setAttribute(Qt::WA_StaticContents);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Client c;
   FrameMonitor mon{&c};
   c.setMinimumSize(200, 200);
   c.show();
   return app.exec();
}
