#include <QApplication>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMetaObject>

class Window : public QWidget {
   Q_OBJECT
   QFormLayout m_layout;
   QPushButton m_button1;
   QPushButton m_button2;
   QPlainTextEdit m_text;
   Q_SLOT void on_button1_clicked() {
      // Here we simulate a `clicked()` emission from m_button2
      // This is for Qt 5
#if QT_VERSION>=QT_VERSION_CHECK(5,0,0)
      emit m_button2.clicked(m_button2.isChecked());
#endif
      m_text.appendPlainText(__FUNCTION__);
   }
   Q_SLOT void on_button2_clicked() {
      m_text.appendPlainText(__FUNCTION__);
   }
public:
   Window(QWidget * parent = 0) :
      QWidget(parent),
      m_layout(this),
      m_button1("Button 1"),
      m_button2("Button 2")
   {
      m_layout.addRow(&m_text);
      m_layout.addRow("Click it ->", &m_button1);
      m_layout.addRow("No, click that too ->", &m_button2);
      m_button1.setObjectName("button1");
      m_button2.setObjectName("button2");
      // In Qt 4, signals are private, so we can't merely emit someone
      // else's signal. We can simply connect one signal to another.
#if QT_VERSION<QT_VERSION_CHECK(5,0,0)
      connect(&m_button1, SIGNAL(clicked(bool)),
              &m_button2, SIGNAL(clicked(bool)));
#endif
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Window w;
   w.show();
   return a.exec();
}

#include "main.moc"
