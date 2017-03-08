// https://github.com/KubaO/stackoverflown/tree/master/questions/statemachine-animation-42682462
#include <QtWidgets>

const char kColor[] = "color";
class Widget : public QWidget {
   Q_OBJECT
   Q_PROPERTY(QColor color MEMBER m_color NOTIFY colorChanged)
   QColor m_color{Qt::blue};
   QStateMachine m_machine{this};
   QState s0{&m_machine}, s1{&m_machine}, s2{&m_machine};
   QEventTransition t01{this, QEvent::MouseButtonPress};
   QPropertyAnimation anim_s1{this, kColor}, anim_s2{this, kColor};
   void paintEvent(QPaintEvent *) override {
      QPainter{this}.fillRect(rect(), m_color);
   }
   Q_SIGNAL void colorChanged(const QColor &);
public:
   Widget() {
      connect(this, &Widget::colorChanged, [this]{ update(); });
      s1.assignProperty(this, kColor, QColor{Qt::red});
      s2.assignProperty(this, kColor, QColor{Qt::green});

      t01.setTargetState(&s1);
      s0.addTransition(&t01);                              t01.addAnimation(&anim_s1);
      s1.addTransition(&s1, &QState::propertiesAssigned, &s2)->addAnimation(&anim_s2);
      s2.addTransition(&s2, &QState::propertiesAssigned, &s1)->addAnimation(&anim_s1);

      anim_s1.setDuration(1000);
      anim_s2.setDuration(2000);

      m_machine.setInitialState(&s0);
      m_machine.start();
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Widget w;
   w.setFixedSize(300, 200);
   w.show();
   return app.exec();
}
#include "main.moc"
