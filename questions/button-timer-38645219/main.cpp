// https://github.com/KubaO/stackoverflown/tree/master/questions/button-timer-38645219

#if 1
#include <QtWidgets>

class Timer : public QObject {
   Q_OBJECT
   QElapsedTimer timer;
public:
   Q_SLOT void start() { timer.start(); }
   Q_SLOT void stop() { emit elapsed(timer.elapsed()); }
   Q_SIGNAL void elapsed(qint64);
};

class Widget : public QWidget {
   Q_OBJECT
   QFormLayout layout{this};
   QPushButton button{"Press Me"};
   QLabel label;
public:
   Widget() {
      layout.addRow(&button);
      layout.addRow(&label);
      connect(&button, &QPushButton::pressed, this, &Widget::pressed);
      connect(&button, &QPushButton::released, this, &Widget::released);
   }
   Q_SIGNAL void pressed();
   Q_SIGNAL void released();
   Q_SLOT void setText(const QString & text) { label.setText(text); }
};

// use Timer and Widget from preceding example
#include <sstream>
#include <string>
#include <functional>

class Controller {
public:
   using callback_t = std::function<void(const std::string&)>;
   Controller(callback_t && callback) : callback{std::move(callback)} {}
   void onElapsed(int ms) {
      std::stringstream s;
      s << "Pressed for " << ms << " ms";
      callback(s.str());
   }
private:
   callback_t callback;
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Timer t;
   Widget w;
   Controller c{ [&](const std::string & s){ w.setText(QString::fromStdString(s)); } };

   QObject::connect(&w, &Widget::pressed, &t, &Timer::start);
   QObject::connect(&w, &Widget::released, &t, &Timer::stop);
   QObject::connect(&t, &Timer::elapsed, [&](qint64 ms) { c.onElapsed(ms); });
   w.show();
   return app.exec();
}
#include "main.moc"
#endif

#if 0
#include <QtWidgets>

class Timer : public QObject {
   Q_OBJECT
   QElapsedTimer timer;
public:
   Q_SLOT void start() { timer.start(); }
   Q_SLOT void stop() { emit elapsed(timer.elapsed()); }
   Q_SIGNAL void elapsed(qint64);
};

class Widget : public QWidget {
   Q_OBJECT
   QFormLayout layout{this};
   QPushButton button{"Press Me"};
   QLabel label;
public:
   Widget() {
      layout.addRow(&button);
      layout.addRow(&label);
      connect(&button, &QPushButton::pressed, this, &Widget::pressed);
      connect(&button, &QPushButton::released, this, &Widget::released);
   }
   Q_SIGNAL void pressed();
   Q_SIGNAL void released();
   Q_SLOT void setText(const QString & text) { label.setText(text); }
};

class Controller : public QObject {
   Q_OBJECT
public:
   Q_SLOT void elapsed(qint64 ms) {
      emit hasText(QStringLiteral("Pressed for %1 ms").arg(ms));
   }
   Q_SIGNAL void hasText(const QString &);
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Timer t;
   Widget w;
   Controller c;

   QObject::connect(&w, &Widget::pressed, &t, &Timer::start);
   QObject::connect(&w, &Widget::released, &t, &Timer::stop);
   QObject::connect(&t, &Timer::elapsed, &c, &Controller::elapsed);
   QObject::connect(&c, &Controller::hasText, &w, &Widget::setText);
   w.show();
   return app.exec();
}
#include "main.moc"
#endif

#if 0
#include <QtWidgets>

class Widget : public QWidget {
   QFormLayout layout{this};
   QPushButton button{"Press Me"};
   QLabel label;
   QElapsedTimer timer;
public:
   Widget() {
      layout.addRow(&button);
      layout.addRow(&label);


   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Widget w;
   w.show();
   return app.exec();
}
#endif

#if 0
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QFormLayout layout{&w};
   QPushButton button{"Press Me"};
   QLabel label;
   layout.addRow(&button);
   layout.addRow(&label);

   QElapsedTimer timer;
   QObject::connect(&button, &QPushButton::pressed, [&]{ timer.start(); });
   QObject::connect(&button, &QPushButton::released, [&]{
      label.setText(QStringLiteral("Pressed for %1 ms").arg(timer.elapsed()));
   });
   w.show();
   return app.exec();
}
#endif
