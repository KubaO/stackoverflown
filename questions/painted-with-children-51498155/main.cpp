// https://github.com/KubaO/stackoverflown/tree/master/questions/painted-with-children-51498155
// This project is compatible with Qt 4 and Qt 5
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

QRectF scaled(const QRectF &rect, qreal scale) {
   auto const center = rect.center();
   auto const w = rect.width()*scale;
   auto const h = rect.height()*scale;
   return {center.x() - w/2.0, center.y() - h/2.0, w, h};
}

QRectF marginAdded(const QRectF &rect, qreal margin) {
   return rect.adjusted(margin, margin, -margin, -margin);
}

const char buttonQSS[] =
      "* { background-color: white; border-width: 2px; border-style:solid; border-color: red;"
      "    border-radius: 3px; padding: 3px; }"
      "*:pressed { padding-left: 5px; padding-top: 5px; background-color: lightGray; }";

class MyWindow : public QWidget {
   Q_OBJECT
   QPushButton m_restoreButton{"Restore Size"};
   QPushButton m_otherButton{"Bar"};
   QGridLayout m_layout{this};
   Q_SLOT void onRestore() { resize(sizeHint()); }
public:
   explicit MyWindow(QWidget *parent = {}) : QWidget(parent) {
      setAttribute(Qt::WA_OpaquePaintEvent);
      m_layout.addItem(new QSpacerItem(0, 100, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 0, 0);
      m_layout.addWidget(&m_restoreButton, 1, 0);
      m_layout.addWidget(&m_otherButton, 1, 1);
      m_restoreButton.setStyleSheet(buttonQSS);
      connect(&m_restoreButton, SIGNAL(clicked(bool)), SLOT(onRestore()));
   }
protected:
   void paintEvent(QPaintEvent *) override {
      qreal const penWidth = 2.0;
      // Cover the area above all the children
      QRectF const area = marginAdded(QRect(0, 0, width(), childrenRect().top() - 10), penWidth);
      QPainter p(this);
      p.fillRect(rect(), Qt::black);
      p.setPen({Qt::green, penWidth});
      p.drawEllipse(scaled(area, 3./3.));
      p.drawEllipse(scaled(area, 2./3.));
      p.drawEllipse(scaled(area, 1./3.));
   }
   QSize sizeHint() const override { return {320, 568}; /* iPhone 5 */ }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MyWindow w;
   w.show();
   return a.exec();
}
#include "main.moc"
