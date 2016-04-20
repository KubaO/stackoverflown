// https://github.com/KubaO/stackoverflown/tree/master/questions/threaded-paint-36748972
#include <QtWidgets>
#include <QtConcurrent>

class Widget : public QWidget {
   Q_OBJECT
   QImage m_image;
   bool m_pendingRender { false };
   Q_SIGNAL void hasNewRender(const QImage &);
   // Must be thread-safe, we can't access the widget directly!
   void paint() {
      QImage image{2048, 2048, QImage::Format_ARGB32_Premultiplied};
      image.fill(Qt::white);
      QPainter p(&image);
      for (int i = 0; i < 100000; ++i) {
         QColor c{rand() % 256, rand() % 256, rand() % 256};
         p.setBrush(c);
         p.setPen(c);
         p.drawRect(rand() % 2048, rand() % 2048, rand() % 100, rand() % 100);
      }
      emit hasNewRender(image);
   }
   void paintEvent(QPaintEvent *) {
      QPainter p(this);
      p.drawImage(0, 0, m_image);
   }
public:
   Widget(QWidget * parent = 0) : QWidget(parent) {
      this->setAttribute(Qt::WA_OpaquePaintEvent);
      setMinimumSize(200, 200);
      connect(this, &Widget::hasNewRender, this, [this](const QImage & img) {
         m_image = img;
         m_pendingRender = false;
         update();
      });
      refresh();
   }
   Q_SLOT void refresh() {
      if (!m_pendingRender) {
         m_pendingRender = true;
         QtConcurrent::run([this] { paint(); });
      }
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Widget w;
   QPushButton button{"Refresh", &w};
   button.connect(&button, &QPushButton::clicked, &w, [&w]{ w.refresh(); });
   w.show();
   return app.exec();
}

#include "main.moc"
