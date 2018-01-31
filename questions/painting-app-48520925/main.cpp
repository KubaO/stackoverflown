// https://github.com/KubaO/stackoverflown/tree/master/questions/painting-app-48520925
#include <QtWidgets>

class PaintCanvas : public QWidget {
   Q_OBJECT
   Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged USER true)
   Q_PROPERTY(bool neverShrink READ neverShrink WRITE setNeverShrink)
   bool m_neverShrink = true;
   QImage m_image;
   QPoint m_lastPos;
   QPen m_pen{{Qt::red}, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
   void indicateChange() {
      emit imageChanged(m_image);
      if (!this->paintingActive()) update();
   }
   void newImage() {
      auto newSize = m_neverShrink ? size().expandedTo(m_image.size()) : size();
      if (newSize == m_image.size())
         return;
      QImage new_image{newSize, QImage::Format_ARGB32_Premultiplied};
      new_image.fill(Qt::white);
      QPainter p{&new_image};
      p.drawImage(0, 0, m_image);
      p.end();
      m_image.swap(new_image);
      indicateChange();
   }
   void paintEvent(QPaintEvent *) override {
      QPainter p{this};
      newImage(); // ensure that there's something to paint
      p.drawImage(0, 0, m_image);
   }
   void mousePressEvent(QMouseEvent *event) override {
      if (isEnabled() && event->buttons() & Qt::RightButton)
         clear();
      if (! isEnabled() || ! (event->buttons() & Qt::LeftButton))
         return;
      m_lastPos = event->pos();
      QPainter p{&m_image};
      p.setPen(m_pen);
      p.drawLine(m_lastPos, m_lastPos+QPoint{1,1});
      indicateChange();
   }
   void mouseMoveEvent(QMouseEvent *event) override {
      if (! isEnabled() || ! (event->buttons() & Qt::LeftButton))
         return;
      QPainter p{&m_image};
      p.setPen(m_pen);
      p.drawLine(m_lastPos, event->pos());
      m_lastPos = event->pos();
      indicateChange();
   }
public:
   PaintCanvas(QWidget * parent = {}) : QWidget(parent) {
      setAttribute(Qt::WA_OpaquePaintEvent);
      setAttribute(Qt::WA_StaticContents);
   }
   Q_SIGNAL void imageChanged(const QImage &);
   QImage image() const { return m_image; }
   Q_SLOT void setImage(const QImage & image) {
      m_image = image;
      indicateChange();
   }
   void setImage(QImage && image) {
      m_image.swap(image);
      indicateChange();
   }
   bool neverShrink() const { return m_neverShrink; }
   void setNeverShrink(bool n) { m_neverShrink = n; }
   Q_SLOT void clear() {
      m_image = {};
      newImage();
   }
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   PaintCanvas canvas;
   canvas.show();
   return app.exec();
}

#include "main.moc"
