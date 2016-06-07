// https://github.com/KubaO/stackoverflown/tree/master/questions/scenestate-37684315
#include <QtWidgets>

void addTransition(QState * src, QObject * eventSource, QEvent::Type type, QAbstractState * dst)
{
   auto transition = new QEventTransition(eventSource, type);
   transition->setTargetState(dst);
   src->addTransition(transition);
}

struct Card : public QGraphicsObject {
   Card() {
      QPixmap pix{128, 128};
      QPainter p{&pix};
      p.setBrush(Qt::white);
      p.drawRect(pix.rect().adjusted(0,0,-1,-1));
      setPixmap(pix);
   }
};

struct Window : public QWidget {
   QHBoxLayout m_layout{this};
   QGraphicsScene m_scene;
   Card m_cards[10];
   QGraphicsView m_view{&m_scene};

   QStateMachine m_mach;
   QState s1{&m_mach};
   QState s2{&m_mach};
   Window() {
      m_layout.addWidget(&m_view);
      for (auto card & : m_cards)
      m_scene.addItem(&m_item);

      // I want to show the view in s1...
      s1.assignProperty(&m_view, "visible", true);
      // and set object1 visible.
      s1.connect(&s1, &QState::entered, [&]{ m_item.show(); });
      // With a mouse click on the scene I've added a transition to s2.
      addTransition(&s1, &m_view, QEvent::MouseButtonPress, &s2);
      // In s2 I want to hide only object1.
      s2.connect(&s2, &QState::entered, [&]{ m_item.hide(); });
      m_mach.setInitialState(&s1);
      m_mach.start();
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Window w;
   w.show();
   return app.exec();
}

#include "main.moc"
