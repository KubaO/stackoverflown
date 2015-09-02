#include <QApplication>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QMetaObject>

class MyUi : public QWidget {
   Q_OBJECT
   QPlainTextEdit m_edit;
   QPushButton m_button;
   QBoxLayout m_layout;
   Q_SLOT void on_pushButton_clicked() { displayInput(); }
   void displayInput() { m_edit.setPlainText("ABC"); }
public:
   MyUi() :
      m_button("Set Text"),
      m_layout(QBoxLayout::TopToBottom, this)
   {
      m_layout.addWidget(&m_edit);
      m_layout.addWidget(&m_button);
      m_button.setObjectName("pushButton");
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   MyUi ui;
   ui.show();
   return app.exec();
}

#include "main.moc"
