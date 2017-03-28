// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-movable-circles-11188261
// main.cpp
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <cmath>

class Circle : public QGraphicsEllipseItem {
   QBrush m_inBrush{Qt::red}, m_outBrush{Qt::lightGray};
public:
   Circle(const QPointF & c) {
      const qreal r = 10.0 + (50.0*qrand())/RAND_MAX;
      setRect({c.x()-r, c.y()-r, 2.0*r, 2.0*r});
      setFlag(QGraphicsItem::ItemIsMovable);
      setFlag(QGraphicsItem::ItemIsFocusable);
      setPen({Qt::red});
      setBrush(m_outBrush);
   }
   void focusInEvent(QFocusEvent *) override { setBrush(m_inBrush); }
   void focusOutEvent(QFocusEvent *) override { setBrush(m_outBrush); }
};

class Painter : public QWidget {
   Q_OBJECT
   QGridLayout m_layout{this};
   QGraphicsView m_view;
   QPushButton m_clear{"Clear"};
   QPushButton m_remove{"Remove"};
   QGraphicsScene m_scene;
public:
   explicit Painter(QWidget *parent = nullptr);
protected:
   Q_SLOT void on_remove_clicked();
   void mousePressEvent(QMouseEvent *event) override;
};

Painter::Painter(QWidget *parent) : QWidget(parent) {
   m_layout.addWidget(&m_view, 0, 0, 1, 2);
   m_layout.addWidget(&m_clear, 1, 0);
   m_layout.addWidget(&m_remove, 1, 1);
   m_remove.setObjectName("remove");
   QMetaObject::connectSlotsByName(this);
   connect(&m_clear, SIGNAL(clicked()), &m_scene, SLOT(clear()));

   m_view.setRenderHint(QPainter::Antialiasing);
   m_view.setScene(&m_scene);
   m_view.setSceneRect(0,0,500,500);
}

void Painter::mousePressEvent(QMouseEvent *event) {
   auto center = m_view.mapToScene(m_view.mapFromParent(event->pos()));
   m_scene.addItem(new Circle(center));
}

void Painter::on_remove_clicked() {
   delete m_scene.focusItem();
}

int main(int argc, char ** argv)
{
   QApplication app{argc, argv};
   Painter p;
   p.show();
   return app.exec();
}

#include "main.moc"
