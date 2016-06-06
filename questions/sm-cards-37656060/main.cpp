// https://github.com/KubaO/stackoverflown/tree/master/questions/sm-cards-37656060
#include <QtWidgets>

class CardItem : public QGraphicsObject {
   Q_OBJECT
   const QRect cardRect { 0, 0, 80, 120 };
   QString m_text;
   QRectF boundingRect() const Q_DECL_OVERRIDE { return cardRect; }
   void paint(QPainter * p, const QStyleOptionGraphicsItem*, QWidget*) {
      p->setRenderHint(QPainter::Antialiasing);
      p->setPen(Qt::black);
      p->setBrush(isSelected() ? Qt::gray : Qt::white);
      p->drawRoundRect(cardRect.adjusted(0, 0, -1, -1), 10, 10);
      p->setFont(QFont("Helvetica", 20));
      p->drawText(cardRect.adjusted(3,3,-3,-3), m_text);
   }
public:
   CardItem(qreal x, qreal y, const QString & text) : m_text(text) {
      moveBy(x, y);
      setFlags(QGraphicsItem::ItemIsSelectable);
   }
};

void on_delay(QState * src, int ms, QAbstractState * dst) {
   auto timer = new QTimer(src);
   timer->setSingleShot(true);
   timer->setInterval(ms);
   QObject::connect(src, &QState::entered, timer, static_cast<void (QTimer::*)()>(&QTimer::start));
   QObject::connect(src, &QState::exited,  timer, &QTimer::stop);
   src->addTransition(timer, SIGNAL(timeout()), dst);
}

class SignalSource : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void sig();
   SignalSource(QObject * parent = Q_NULLPTR) : QObject(parent) {}
};

void on_selected(QState * src, QGraphicsScene * scene, bool selected, QAbstractState * dst) {
   auto signalSource = new SignalSource(src);
   QObject::connect(scene, &QGraphicsScene::selectionChanged, signalSource, [=] {
      if (scene->selectedItems().isEmpty() == !selected) emit signalSource->sig();
   });
   src->addTransition(signalSource, SIGNAL(sig()), dst);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QGraphicsScene scene;
   QGraphicsView view{&scene};
   scene.addItem(new CardItem(0, 0, "A"));
   scene.addItem(new CardItem(20, 0, "B"));

   QStateMachine machine;
   QState s_idle{&machine};     // idle - no card selected
   QState s_selected{&machine}; // card selected, waiting 1/2 second
   QState s_ready{&machine};    // ready with card selected
   machine.setInitialState(&s_idle);
   on_selected(&s_idle, &scene, true, &s_selected);
   on_delay(&s_selected, 500, &s_ready);
   on_selected(&s_selected, &scene, false, &s_idle);
   on_selected(&s_ready, &scene, false, &s_idle);
   QObject::connect(&s_ready, &QState::entered, &scene, &QGraphicsScene::clearSelection);
   machine.start();

   view.show();
   return app.exec();
}

#include "main.moc"
