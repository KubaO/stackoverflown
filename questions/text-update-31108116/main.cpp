#if 1
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QWidget window;
   QVBoxLayout layout(&window);
   QLineEdit name;
   QTextEdit text;
   layout.addWidget(&name);
   layout.addWidget(&text);
   QObject::connect(&name, &QLineEdit::textChanged, [&](const QString & name){
      text.setPlainText(QString("The name is %1, age 24").arg(name));
   });
   window.show();
   return app.exec();
}
#endif

#if 0
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QApplication>

class Window : public QWidget {
   Q_OBJECT
   QVBoxLayout m_layout; // not a pointer!
   QLineEdit m_name; // not a pointer, must come after the layout!
   QTextEdit m_text;
   Q_SLOT void on_name_textChanged(const QString & name) {
      m_text.setPlainText(QString("The name is %1, age 24.").arg(name));
   }
public:
   Window() : m_layout(this) {
      m_layout.addWidget(&m_name);
      m_layout.addWidget(&m_text);
      m_name.setObjectName("name");
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   Window window;
   window.show();
   return app.exec();
}

#include "main.moc"

#endif

