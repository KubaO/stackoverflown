#include <QMainWindow>
#include <QDockWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDebug>

class Keyboard : public QDockWidget {
   Q_OBJECT
   void init() {
      auto w = new QWidget;
      auto l = new QGridLayout(w);
      for (int i = 0; i < 10; ++ i) {
         auto btn = new QToolButton;
         btn->setText(QString::number(i));
         btn->setProperty("key", Qt::Key_0 + i);
         l->addWidget(btn, 0, i, 1, 1);
         connect(btn, SIGNAL(clicked()), SLOT(clicked()));
      }
      setWidget(w);
      setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
      for (auto o : widget()->children()) {
         if (o->isWidgetType()) static_cast<QWidget*>(o)->setFocusPolicy(Qt::NoFocus);
      }
   }
   void sendKey(Qt::Key key, Qt::KeyboardModifier mod)
   {
      if (! parentWidget()) return;
      auto target = parentWidget()->focusWidget();
      if (! target) return;

      auto repr = QKeySequence(key).toString();
      auto pressEvent = new QKeyEvent(QEvent::KeyPress, key, mod, repr);
      auto releaseEvent = new QKeyEvent(QEvent::KeyRelease, key, mod, repr);
      qApp->postEvent(target, pressEvent);
      qApp->postEvent(target, releaseEvent);
      qDebug() << repr;
   }
   Q_SLOT void clicked() {
      auto key = sender()->property("key");
      if (key.isValid()) sendKey((Qt::Key)key.toInt(), Qt::NoModifier);
   }
public:
   Keyboard(const QString & title, QWidget *parent = 0) :
      QDockWidget(title, parent) { init(); }
   explicit Keyboard(QWidget *parent = 0) : QDockWidget(parent) { init(); }
};

int main(int argc, char ** argv)
{
   QApplication a(argc, argv);
   QMainWindow w;
   w.setCentralWidget(new QLineEdit);
   w.addDockWidget(Qt::TopDockWidgetArea, new Keyboard("Keyboard", &w));
   w.show();
   return a.exec();
}

#include "main.moc"
