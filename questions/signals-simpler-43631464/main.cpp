// https://github.com/KubaO/stackoverflown/tree/master/questions/signals-simpler-43631464
#include <QtWidgets>
#include <initializer_list>

class Receiver : public QLabel {
   Q_OBJECT
public:
   Receiver(QWidget * parent = {}) : QLabel{parent} {}
   Q_SLOT void intSlot(int val) {
      setText(QStringLiteral("int = %1").arg(val));
   }
};

class Sender : public QWidget {
   Q_OBJECT
   QFormLayout m_layout{this};
   QPushButton btn1{"Send 1"}, btn2{"Send 5"}, btn3{"Send 10"};
public:
   Sender(QWidget * parent = {}) : QWidget{parent} {
      m_layout.setMargin(1);
      auto const buttons = {&btn1, &btn2, &btn3};
      for (auto b : buttons) m_layout.addWidget(b);
      // Approach 1
      auto const clicked = &QPushButton::clicked;
      connect(&btn1, clicked, this, [this]{ emit signal1(1); });
      connect(&btn2, clicked, this, [this]{ emit signal2(5); });
      connect(&btn3, clicked, this, [this]{ emit signal3(10); });
      // Approach 2
      auto const cClicked = [this](QPushButton * btn, std::function<void()> fun) {
         connect(btn, &QPushButton::clicked, this, fun);
      };
      cClicked(&btn1, [=]{ emit signal4("One"); });
      cClicked(&btn2, [=]{ emit signal5("Five"); });
      cClicked(&btn3, [=]{ emit signal6("Ten"); });
      // Approach 3
      for (auto b : buttons)
         connect(b, &QPushButton::clicked, [this, b]{ emit signal7(b->text()); });
   }
   Q_SIGNAL void signal1(int);
   Q_SIGNAL void signal2(int);
   Q_SIGNAL void signal3(int);
   Q_SIGNAL void signal4(const QString &);
   Q_SIGNAL void signal5(const QString &);
   Q_SIGNAL void signal6(const QString &);
   Q_SIGNAL void signal7(const QString &);
};

using Widgets = std::initializer_list<QWidget*>;

int main(int argc, char **argv)
{
   QApplication app{argc, argv};
   QWidget win;
   QVBoxLayout layout{&win};
   Sender sender;
   Receiver receiver;
   for (auto w : Widgets{&sender, &receiver}) layout.addWidget(w);

   // Factor out connection
   auto const cIntSlot = [&](void (Sender::*signal)(int)){
      QObject::connect(&sender, signal, &receiver, &Receiver::intSlot);
   };
   // Factor out connection on a list - various approached
   for (auto signal : {&Sender::signal1, &Sender::signal2, &Sender::signal3})
      cIntSlot(signal);
   for (auto signal: {&Sender::signal4, &Sender::signal5, &Sender::signal6})
      QObject::connect(&sender, signal, &receiver, &Receiver::stringSlot);
   QObject::connect(&Sender::signal7, &receiver, &Receiver::)

   win.show();
   return app.exec();
}
#include "main.moc"
