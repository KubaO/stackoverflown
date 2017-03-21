// https://github.com/KubaO/stackoverflown/tree/master/questions/vkb-focus-18558664
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class Keyboard : public QDockWidget {
   Q_OBJECT
   QWidget m_widget;
   QGridLayout m_layout{&m_widget};
   QToolButton m_buttons[10];
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
   explicit Keyboard(const QString & title, QWidget *parent = nullptr) : Keyboard(parent) {
      setWindowTitle(title);
   }
   explicit Keyboard(QWidget *parent = nullptr) : QDockWidget(parent) {
      int i{};
      for (auto & btn : m_buttons) {
         btn.setText(QString::number(i));
         btn.setProperty("key", Qt::Key_0 + i);
         m_layout.addWidget(&btn, 0, i, 1, 1);
         connect(&btn, SIGNAL(clicked()), SLOT(clicked()));
         btn.setFocusPolicy(Qt::NoFocus);
         ++i;
      }
      setWidget(&m_widget);
      setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
   }
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
