// https://github.com/KubaO/stackoverflown/tree/master/questions/widget-pen-wide-38019846
#include <QtWidgets>

class Widget : public QWidget {
   Q_OBJECT
   Q_PROPERTY(qreal penWidth READ penWidth WRITE setPenWidth)
   qreal m_penWidth = 1.0;
protected:
   void paintEvent(QPaintEvent *) override {
      QPainter p{this};
      p.setPen({Qt::black, m_penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin});
      p.setBrush(Qt::cyan);
      qreal d = m_penWidth/2.0;
      p.drawRect(QRectF{d, d, width()-m_penWidth, height()-m_penWidth});
   }
public:
   explicit Widget(QWidget * parent = 0) : QWidget{parent} { }
   qreal penWidth() const { return m_penWidth; }
   void setPenWidth(qreal width) {
      if (width == m_penWidth) return;
      m_penWidth = width;
      update();
   }
   QSize sizeHint() const override { return {100, 100}; }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget top;
   QVBoxLayout layout{&top};
   Widget widget;
   QSlider slider{Qt::Horizontal};
   layout.addWidget(&widget);
   layout.addWidget(&slider);

   slider.setMinimum(100);
   slider.setMaximum(1000);
   QObject::connect(&slider, &QSlider::valueChanged, [&](int val){
      widget.setPenWidth(val/100.0);
   });

   top.show();
   return app.exec();
}

#include "main.moc"
